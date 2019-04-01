#include "dem.h"
#include "track.h"


int Track::_elevationWindow = 3;
int Track::_speedWindow = 5;
int Track::_heartRateWindow = 3;
int Track::_cadenceWindow = 3;
int Track::_powerWindow = 3;

qreal Track::_pauseSpeed = 0.5;
int Track::_pauseInterval = 10;

bool Track::_outlierEliminate = true;
bool Track::_useReportedSpeed = false;
bool Track::_useDEM = false;


static qreal median(QVector<qreal> &v)
{
	qSort(v.begin(), v.end());
	return v.at(v.size() / 2);
}

static qreal MAD(QVector<qreal> &v, qreal m)
{
	for (int i = 0; i < v.size(); i++)
		v[i] = qAbs(v.at(i) - m);
	qSort(v.begin(), v.end());
	return v.at(v.size() / 2);
}

static QSet<int> eliminate(const QVector<qreal> &v)
{
	QSet<int> rm;

	QVector<qreal> w(v);
	qreal m = median(w);
	qreal M = MAD(w, m);

	for (int i = 0; i < v.size(); i++)
		if (qAbs((0.6745 * (v.at(i) - m)) / M) > 5)
			rm.insert(i);

	return rm;
}

static GraphSegment filter(const GraphSegment &g, int window)
{
	if (g.size() < window || window < 2)
		return GraphSegment(g);

	qreal acc = 0;
	GraphSegment ret(g.size());

	for (int i = 0; i < window; i++)
		acc += g.at(i).y();
	for (int i = 0; i <= window/2; i++)
		ret[i] = GraphPoint(g.at(i).s(), g.at(i).t(), acc/window);

	for (int i = window/2 + 1; i < g.size() - window/2; i++) {
		acc += g.at(i + window/2).y() - g.at(i - (window/2 + 1)).y();
		ret[i] = GraphPoint(g.at(i).s(), g.at(i).t(), acc/window);
	}

	for (int i = g.size() - window/2; i < g.size(); i++)
		ret[i] = GraphPoint(g.at(i).s(), g.at(i).t(), acc/window);

	return ret;
}


Track::Track(const TrackData &data) : _data(data), _pause(0)
{
	qreal ds, dt;

	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.isEmpty())
			continue;

		// precompute distances, times, speeds and acceleration
		QVector<qreal> acceleration;

		_segments.append(Segment());
		Segment &seg = _segments.last();

		seg.distance.append(i ? _segments.at(i-1).distance.last() : 0);
		seg.time.append(i ? _segments.at(i-1).time.last() :
		  sd.first().hasTimestamp() ? 0 : NAN);
		seg.speed.append(sd.first().hasTimestamp() ? 0 : NAN);
		acceleration.append(sd.first().hasTimestamp() ? 0 : NAN);

		for (int j = 1; j < sd.size(); j++) {
			ds = sd.at(j).coordinates().distanceTo(
			  sd.at(j-1).coordinates());
			seg.distance.append(seg.distance.last() + ds);

			if (sd.at(j).timestamp() >= sd.at(j-1).timestamp())
				dt = sd.at(j-1).timestamp().msecsTo(
				  sd.at(j).timestamp()) / 1000.0;
			else
				dt = NAN;
			seg.time.append(seg.time.last() + dt);

			if (dt < 1e-3) {
				seg.speed.append(seg.speed.last());
				acceleration.append(acceleration.last());
			} else {
				qreal v = ds / dt;
				qreal dv = v - seg.speed.last();
				seg.speed.append(v);
				acceleration.append(dv / dt);
			}
		}

		// get stop-points + pause duration
		for (int j = 1; j < seg.time.size(); j++) {
			if (seg.time.at(j) > seg.time.at(j-1) + _pauseInterval
			  && seg.speed.at(j) < _pauseSpeed) {
				_pause += seg.time.at(j) - seg.time.at(j-1);
				seg.stop.insert(j-1);
				seg.stop.insert(j);
			}
		}

		if (!_outlierEliminate)
			continue;


		// eliminate outliers
		seg.outliers = eliminate(acceleration);

		// stop-points can not be outliers
		QSet<int>::const_iterator it;
		for (it = seg.stop.constBegin(); it != seg.stop.constEnd(); ++it)
			seg.outliers.remove(*it);

		// recompute distances (and dependand data) without outliers
		int last = 0;
		for (int j = 0; j < sd.size(); j++) {
			if (seg.outliers.contains(j))
				last++;
			else
				break;
		}
		for (int j = last + 1; j < sd.size(); j++) {
			if (seg.outliers.contains(j))
				continue;
			if (discardStopPoint(seg, j)) {
				seg.distance[j] = seg.distance.at(last);
				seg.speed[j] = 0;
			} else {
				ds = sd.at(j).coordinates().distanceTo(
				  sd.at(last).coordinates());
				seg.distance[j] = seg.distance.at(last) + ds;

				dt = seg.time.at(j) - seg.time.at(last);
				seg.speed[j] = (dt < 1e-3) ? seg.speed.at(last) : ds / dt;
			}
			last = j;
		}
	}
}

Graph Track::elevation() const
{
	Graph ret;

	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.size() < 2)
			continue;
		const Segment &seg = _segments.at(i);
		GraphSegment gs;

		for (int j = 0; j < sd.size(); j++) {
			if (seg.outliers.contains(j))
				continue;

			if (sd.at(j).hasElevation() && !_useDEM)
				gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j),
				  sd.at(j).elevation()));
			else {
				qreal elevation = DEM::elevation(sd.at(j).coordinates());
				if (!std::isnan(elevation))
					gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j),
					  elevation));
				else if (sd.at(j).hasElevation())
					gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j),
					  sd.at(j).elevation()));
			}
		}

		ret.append(filter(gs, _elevationWindow));
	}

	return ret;
}

Graph Track::speed() const
{
	Graph ret;

	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.size() < 2)
			continue;
		const Segment &seg = _segments.at(i);
		GraphSegment gs;
		QList<int> stop;
		qreal v;

		for (int j = 0; j < sd.size(); j++) {
			if (seg.stop.contains(j) && (!std::isnan(seg.speed.at(j))
			  || sd.at(j).hasSpeed())) {
				v = 0;
				stop.append(gs.size());
			} else if (_useReportedSpeed && sd.at(j).hasSpeed()
			  && !seg.outliers.contains(j))
				v = sd.at(j).speed();
			else if (!std::isnan(seg.speed.at(j)) && !seg.outliers.contains(j))
				v = seg.speed.at(j);
			else
				continue;

			gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j), v));
		}

		ret.append(filter(gs, _speedWindow));
		GraphSegment &filtered = ret.last();

		for (int j = 0; j < stop.size(); j++)
			filtered[stop.at(j)].setY(0);
	}

	return ret;
}

Graph Track::heartRate() const
{
	Graph ret;

	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.size() < 2)
			continue;
		const Segment &seg = _segments.at(i);
		GraphSegment gs;

		for (int j = 0; j < sd.size(); j++)
			if (sd.at(j).hasHeartRate() && !seg.outliers.contains(j))
				gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j),
				  sd.at(j).heartRate()));

		ret.append(filter(gs, _heartRateWindow));
	}

	return ret;
}

Graph Track::temperature() const
{
	Graph ret;

	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.size() < 2)
			continue;
		const Segment &seg = _segments.at(i);
		GraphSegment gs;

		for (int j = 0; j < sd.count(); j++) {
			if (sd.at(j).hasTemperature() && !seg.outliers.contains(j))
				gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j),
				  sd.at(j).temperature()));
		}

		ret.append(gs);
	}

	return ret;
}

Graph Track::ratio() const
{
	Graph ret;

	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.size() < 2)
			continue;
		const Segment &seg = _segments.at(i);
		GraphSegment gs;

		for (int j = 0; j < sd.size(); j++)
			if (sd.at(j).hasRatio() && !seg.outliers.contains(j))
				gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j),
				  sd.at(j).ratio()));

		ret.append(gs);
	}

	return ret;
}

Graph Track::cadence() const
{
	Graph ret;

	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.size() < 2)
			continue;
		const Segment &seg = _segments.at(i);
		GraphSegment gs;
		QList<int> stop;
		qreal c;

		for (int j = 0; j < sd.size(); j++) {
			if (sd.at(j).hasCadence() && seg.stop.contains(j)) {
				c = 0;
				stop.append(gs.size());
			} else if (sd.at(j).hasCadence() && !seg.outliers.contains(j))
				c = sd.at(j).cadence();
			else
				continue;

			gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j), c));
		}

		ret.append(filter(gs, _cadenceWindow));
		GraphSegment &filtered = ret.last();

		for (int j = 0; j < stop.size(); j++)
			filtered[stop.at(j)].setY(0);
	}

	return ret;
}

Graph Track::power() const
{
	Graph ret;
	QList<int> stop;
	qreal p;


	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.size() < 2)
			continue;
		const Segment &seg = _segments.at(i);
		GraphSegment gs;

		for (int j = 0; j < sd.size(); j++) {
			if (sd.at(j).hasPower() && seg.stop.contains(j)) {
				p = 0;
				stop.append(gs.size());
			} else if (sd.at(j).hasPower() && !seg.outliers.contains(j))
				p = sd.at(j).power();
			else
				continue;

			gs.append(GraphPoint(seg.distance.at(j), seg.time.at(j), p));
		}

		ret.append(filter(gs, _powerWindow));
		GraphSegment &filtered = ret.last();

		for (int j = 0; j < stop.size(); j++)
			filtered[stop.at(j)].setY(0);
	}

	return ret;
}

qreal Track::distance() const
{
	for (int i = _segments.size() - 1; i >= 0; i--) {
		const Segment &seg = _segments.at(i);

		for (int j = seg.distance.size() - 1; j >= 0; j--)
			if (!seg.outliers.contains(j))
				return seg.distance.at(j);
	}

	return 0;
}

qreal Track::time() const
{
	for (int i = _segments.size() - 1; i >= 0; i--) {
		const Segment &seg = _segments.at(i);

		for (int j = seg.time.size() - 1; j >= 0; j--)
			if (!seg.outliers.contains(j))
				return seg.time.at(j);
	}

	return 0;
}

qreal Track::movingTime() const
{
	return (time() - _pause);
}

QDateTime Track::date() const
{
	return (_data.size() && _data.first().size())
	  ? _data.first().first().timestamp() : QDateTime();
}

Path Track::path() const
{
	Path ret;

	for (int i = 0; i < _data.size(); i++) {
		const SegmentData &sd = _data.at(i);
		if (sd.size() < 2)
			continue;
		const Segment &seg = _segments.at(i);
		ret.append(PathSegment());
		PathSegment &ps = ret.last();

		for (int j = 0; j < sd.size(); j++)
			if (!seg.outliers.contains(j) && !discardStopPoint(seg, j))
				ps.append(PathPoint(sd.at(j).coordinates(),
				  seg.distance.at(j)));
	}

	return ret;
}

bool Track::discardStopPoint(const Segment &seg, int i) const
{
	return (seg.stop.contains(i) && seg.stop.contains(i-1)
	  && seg.stop.contains(i+1) && i > 0 && i < seg.distance.size() - 1);
}

bool Track::isValid() const
{
	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).size() >= 2)
			return true;
	return false;
}
