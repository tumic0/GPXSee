#include <QImage>
#include "style.h"

#define TYPE(t) ((t)<<8)

void Style::defaultPolygonStyle()
{
	_polygons[TYPE(0x01)] = Style::Polygon(QBrush("#dfd3b5"));
	_polygons[TYPE(0x02)] = Style::Polygon(QBrush("#dfd3b5"));
	_polygons[TYPE(0x03)] = Style::Polygon(QBrush("#dfd3b5"));
	_polygons[TYPE(0x04)] = Style::Polygon(QBrush("#ff4040", Qt::BDiagPattern),
	  QPen(QColor("#ff4040")));
	_polygons[TYPE(0x05)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x06)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x07)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x08)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x09)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x0a)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x0b)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x0c)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x0d)] = Style::Polygon(QBrush("#f8e3be"));
	_polygons[TYPE(0x0e)] = Style::Polygon(QBrush("#ffffff"));
	_polygons[TYPE(0x0f)] = Style::Polygon(QBrush("#e6e2d9"));
	_polygons[TYPE(0x10)] = Style::Polygon(QBrush("#e6e2d9"));
	_polygons[TYPE(0x11)] = Style::Polygon(QBrush("#e6e2d9"));
	_polygons[TYPE(0x12)] = Style::Polygon(QBrush("#e6e2d9"));
	_polygons[TYPE(0x13)] = Style::Polygon(QBrush("#dbd0b6"),
	  QPen(QColor("#cdccc4"), 1));
	_polygons[TYPE(0x14)] = Style::Polygon(QBrush(Qt::NoBrush),
	  QPen(QColor("#9ac269"), 2, Qt::DashLine));
	_polygons[TYPE(0x15)] = Style::Polygon(QBrush(Qt::NoBrush),
	  QPen(QColor("#9ac269"), 2, Qt::DashLine));
	_polygons[TYPE(0x17)] = Style::Polygon(QBrush("#d4ebb8"));
	_polygons[TYPE(0x18)] = Style::Polygon(QBrush("#d4ebb8"));
	_polygons[TYPE(0x19)] = Style::Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x1a)] = Style::Polygon(QBrush("#000000", Qt::Dense6Pattern),
	  QPen(QColor("#cdccc4"), 1));
	_polygons[TYPE(0x1e)] = Style::Polygon(QBrush(Qt::NoBrush),
	  QPen(QColor("#9ac269"), 1, Qt::DashLine));
	_polygons[TYPE(0x1f)] = Style::Polygon(QBrush(Qt::NoBrush),
	  QPen(QColor("#9ac269"), 1, Qt::DashLine));
	_polygons[TYPE(0x28)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x29)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x32)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3b)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3c)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3d)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3e)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3f)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x40)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x41)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x42)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x43)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x44)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x45)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x46)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x47)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x48)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x49)] = Style::Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x4b)] = Style::Polygon(QBrush("#f1f0e5"), QPen("#f1f0e5"));
	_polygons[TYPE(0x4a)] = Style::Polygon(QBrush("#f1f0e5"), QPen("#f1f0e5"));
	_polygons[TYPE(0x4c)] = Style::Polygon(QBrush("#9fc4e1", Qt::Dense6Pattern));
	_polygons[TYPE(0x4d)] = Style::Polygon(QBrush("#ddf1fd"));
	_polygons[TYPE(0x4e)] = Style::Polygon(QBrush("#e3edc1"));
	_polygons[TYPE(0x4f)] = Style::Polygon(QBrush("#d4ebb8"));
	_polygons[TYPE(0x50)] = Style::Polygon(QBrush("#d4ebb8"));
	_polygons[TYPE(0x51)] = Style::Polygon(QBrush("#9fc4e1", Qt::Dense4Pattern));
	_polygons[TYPE(0x52)] = Style::Polygon(QBrush("#d4ebb8"));

	_drawOrder << TYPE(0x4b) << TYPE(0x4a) << TYPE(0x01) << TYPE(0x02)
	  << TYPE(0x03) << TYPE(0x17) << TYPE(0x18) << TYPE(0x19) << TYPE(0x1a)
	  << TYPE(0x28) << TYPE(0x29) << TYPE(0x32) << TYPE(0x3b) << TYPE(0x3c)
	  << TYPE(0x3d) << TYPE(0x3e) << TYPE(0x3f) << TYPE(0x40) << TYPE(0x41)
	  << TYPE(0x42) << TYPE(0x43) << TYPE(0x44) << TYPE(0x45) << TYPE(0x46)
	  << TYPE(0x47) << TYPE(0x48) << TYPE(0x49) << TYPE(0x4c) << TYPE(0x4d)
	  << TYPE(0x4e) << TYPE(0x4f) << TYPE(0x50) << TYPE(0x51) << TYPE(0x52)
	  << TYPE(0x14) << TYPE(0x15) << TYPE(0x1e) << TYPE(0x1f) << TYPE(0x04)
	  << TYPE(0x05) << TYPE(0x06) << TYPE(0x07) << TYPE(0x08) << TYPE(0x09)
	  << TYPE(0x0a) << TYPE(0x0b) << TYPE(0x0c) << TYPE(0x0d) << TYPE(0x0e)
	  << TYPE(0x0f) << TYPE(0x10) << TYPE(0x11) << TYPE(0x12) << TYPE(0x13);
}

void Style::defaultLineStyle()
{
	QVector<qreal> pattern;
	pattern << 4 << 4;
	QPen rr(QColor("#717171"), 3, Qt::CustomDashLine);
	rr.setDashPattern(pattern);

	_lines[TYPE(0x01)] = Style::Line(QPen(QColor("#9bd772"), 2, Qt::SolidLine),
	  QPen(QColor("#72a35a"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x02)] = Style::Line(QPen(QColor("#ffcc78"), 2, Qt::SolidLine),
	  QPen(QColor("#e8a541"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x03)] = Style::Line(QPen(QColor("#ffcc78"), 2, Qt::SolidLine),
	  QPen(QColor("#e8a541"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x04)] = Style::Line(QPen(QColor("#faef75"), 3, Qt::SolidLine),
	  QPen(QColor("#dbd27b"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x05)] = Style::Line(QPen(QColor("#ffffff"), 3, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x06)] = Style::Line(QPen(QColor("#ffffff"), 3, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x07)] = Style::Line(QPen(QColor("#ffffff"), 2, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x08)] = Style::Line(QPen(QColor("#ffcc78"), 2, Qt::SolidLine),
	  QPen(QColor("#e8a541"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x09)] = Style::Line(QPen(QColor("#9bd772"), 2, Qt::SolidLine),
	  QPen(QColor("#72a35a"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x0a)] = Style::Line(QPen(QColor("#aba083"), 1, Qt::DashLine));
	_lines[TYPE(0x0b)] = Style::Line(QPen(QColor("#ffcc78"), 2, Qt::SolidLine),
	  QPen(QColor("#e8a541"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x0c)] = Style::Line(QPen(QColor("#ffffff"), 3, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x14)] = Style::Line(rr, QPen(Qt::white, 3, Qt::SolidLine,
	  Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x16)] = Style::Line(QPen(QColor("#aba083"), 1, Qt::DotLine));
	_lines[TYPE(0x18)] = Style::Line(QPen(QColor("#9fc4e1"), 2, Qt::SolidLine));
	_lines[TYPE(0x18)].setTextColor(QColor("#9fc4e1"));
	//_lines[TYPE(0x1a)] = Style::Line(QPen(QColor("#7697b7"), 1, Qt::DashLine));
	_lines[TYPE(0x1b)] = Style::Line(QPen(QColor("#7697b7"), 1, Qt::DashLine));
	_lines[TYPE(0x1e)] = Style::Line(QPen(QColor("#505145"), 2, Qt::DashDotLine));
	_lines[TYPE(0x1f)] = Style::Line(QPen(QColor("#9fc4e1"), 3, Qt::SolidLine));
	_lines[TYPE(0x1f)].setTextColor(QColor("#9fc4e1"));
	_lines[TYPE(0x21)] = Style::Line(QPen(QColor("#cacfc0"), 1, Qt::SolidLine));
	_lines[TYPE(0x21)].setTextColor(QColor("#62695a"));
	_lines[TYPE(0x21)].setTextFontSize(Style::Small);
	_lines[TYPE(0x22)] = Style::Line(QPen(QColor("#cacfc0"), 1.5, Qt::SolidLine));
	_lines[TYPE(0x22)].setTextColor(QColor("#62695a"));
	_lines[TYPE(0x22)].setTextFontSize(Style::Small);
	_lines[TYPE(0x24)] = Style::Line(QPen(QColor("#55aaff"), 1, Qt::SolidLine));
	_lines[TYPE(0x24)].setTextColor(QColor("#55aaff"));
	_lines[TYPE(0x24)].setTextFontSize(Style::Small);
	_lines[TYPE(0x25)] = Style::Line(QPen(QColor("#55aaff"), 1.5, Qt::SolidLine));
	_lines[TYPE(0x25)].setTextColor(QColor("#55aaff"));
	_lines[TYPE(0x25)].setTextFontSize(Style::Small);
	_lines[TYPE(0x26)] = Style::Line(QPen(QColor("#9fc4e1"), 2, Qt::DotLine));
	_lines[TYPE(0x27)] = Style::Line(QPen(QColor("#ffffff"), 4, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	//_lines[TYPE(0x28)] = Style::Line(QPen(QColor("#5a5a5a"), 1, Qt::SolidLine));
	_lines[TYPE(0x29)] = Style::Line(QPen(QColor("#5a5a5a"), 1, Qt::SolidLine));
	_lines[TYPE(0x29)].setTextFontSize(Style::None);
}

static bool readBitmap(SubFile *file, SubFile::Handle &hdl, QImage &img,
  int bpp)
{
	if (!bpp)
		return true;

	for (int y = 0; y < img.height(); y++) {
		for (int x = 0; x < img.width(); x += 8/bpp) {
			quint8 color;
			if (!file->readByte(hdl, color))
				return false;

			for (int i = 0; i < 8/bpp && x + i < img.width(); i++) {
				int value = (i > 0) ? (color >>= bpp) : color;

				if (bpp == 4)
					value = value & 0xf;
				else if (bpp == 2)
					value = value & 0x3;
				else if (bpp == 1)
					value = value & 0x1;

				img.setPixel(x + i, y, value);
			}
		}
	}

	return true;
}

static bool readColor(SubFile *file, SubFile::Handle &hdl, QColor &color)
{
	quint8 b, g, r;

	if (!(file->readByte(hdl, b) && file->readByte(hdl, g)
	  && file->readByte(hdl, r)))
		return false;

	color = qRgb(r, g, b);

	return true;
}

static bool skipLocalization(SubFile *file, SubFile::Handle &hdl)
{
	quint8 t8, n = 1;
	quint16 len;

	if (!file->readByte(hdl, t8))
		return false;
	len = t8;
	if (!(t8 & 0x01)) {
		n = 2;
		if (!file->readByte(hdl, t8))
			return false;
		len |= t8 << 8;
	}

	len -= n;
	while (len > 0) {
		if (!file->readByte(hdl, t8))
			return false;
		len -= 2 * n;
		while (len > 0) {
			if (!file->readByte(hdl, t8))
				return false;
			len -= 2 * n;
			if (!t8)
				break;
		}
	}

	return true;
}


bool Style::itemInfo(SubFile *file, SubFile::Handle &hdl,
  const Section &section, ItemInfo &info)
{
	quint16 t16_1, t16_2;
	quint8 t8;

	if (section.arrayItemSize == 5) {
		if (!(file->readUInt16(hdl, t16_1) && file->readUInt16(hdl, t16_2)
		  && file->readByte(hdl, t8)))
			return false;
		info.offset = t16_2 | (t8<<16);
	} else if (section.arrayItemSize == 4) {
		if (!(file->readUInt16(hdl, t16_1) && file->readUInt16(hdl, t16_2)))
			return false;
		info.offset = t16_2;
	} else if (section.arrayItemSize == 3) {
		if (!(file->readUInt16(hdl, t16_1) && file->readByte(hdl, t8)))
			return false;
		info.offset = t8;
	} else
		return false;

	t16_2 = (t16_1 >> 5) | (( t16_1 & 0x1f) << 11);
	info.type = t16_2 & 0x7F;
	info.subtype  = t16_1 & 0x1F;
	info.extended = t16_1 & 0x2000;

	return true;
}

bool Style::parsePolygons(SubFile *file, SubFile::Handle &hdl,
  const Section &section)
{
	for (quint32 i = 0; i < section.arraySize / section.arrayItemSize; i++) {
		if (!file->seek(hdl, section.arrayOffset + i * section.arrayItemSize))
			return false;
		ItemInfo info;
		if (!itemInfo(file, hdl, section, info))
			return false;
		quint32 type = info.extended
		  ? 0x10000 | (info.type << 8) | info.subtype : (info.type << 8);

		quint8 t8, flags;
		if (!(file->seek(hdl, section.offset + info.offset)
		  && file->readByte(hdl, t8)))
			return false;
		flags = t8 & 0x0F;

		QColor c1, c2, c3, c4;
		QImage img(32, 32, QImage::Format_Indexed8);

		switch (flags) {
			case 0x01:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3) && readColor(file, hdl, c4)))
					return false;
				_polygons[type] = Style::Polygon(QBrush(c1), QPen(c3, 2));
				break;

			case 0x06:
			case 0x07:
				if (!readColor(file, hdl, c1))
					return false;
				_polygons[type] = Style::Polygon(QBrush(c1));
				break;

			case 0x08:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)))
					return false;

				img.setColorCount(2);
				img.setColor(0, c2.rgb());
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Style::Polygon(QBrush(img));
				break;

			case 0x09:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3) && readColor(file, hdl, c4)))
					return false;

				img.setColorCount(2);
				img.setColor(0, c2.rgb());
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Style::Polygon(QBrush(img));
				break;

			case 0x0B:
			case 0x0D:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3)))
					return false;

				img.setColorCount(2);
				img.setColor(0, (flags == 0x0B) ? qRgba(255, 255, 255, 0)
				  : c2.rgb());
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Style::Polygon(QBrush(img));
				break;

			case 0x0E:
				if (!readColor(file, hdl, c1))
					return false;

				img.setColorCount(2);
				img.setColor(0, qRgba(255, 255, 255, 0));
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Style::Polygon(QBrush(img));
				break;

			case 0x0F:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)))
					return false;

				img.setColorCount(2);
				img.setColor(0, qRgba(255, 255, 255, 0));
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Style::Polygon(QBrush(img));
				break;

			default:
				return false;
		}
	}

	return true;
}

bool Style::parseLines(SubFile *file, SubFile::Handle &hdl,
  const Section &section)
{
	for (quint32 i = 0; i < section.arraySize / section.arrayItemSize; i++) {
		if (!file->seek(hdl, section.arrayOffset + i * section.arrayItemSize))
			return false;
		ItemInfo info;
		if (!itemInfo(file, hdl, section, info))
			return false;
		quint32 type = info.extended
		  ? 0x10000 | (info.type << 8) | info.subtype : (info.type << 8);

		quint8 t8_1, t8_2, flags, rows;
		if (!(file->seek(hdl, section.offset + info.offset)
		  && file->readByte(hdl, t8_1) && file->readByte(hdl, t8_2)))
			return false;
		flags = t8_1 & 0x07;
		rows = t8_1 >> 3;
		bool localization = t8_2 & 0x01;
		bool textColor = t8_2 & 0x04;

		QColor c1, c2, c3, c4;
		quint8 w1, w2;

		switch (flags) {
			case 0x00:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, c2.rgb());
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Style::Line(img);
				} else {
					if (!(file->readByte(hdl, w1) && file->readByte(hdl, w2)))
						return false;

					_lines[type] = (w2 > w1)
					  ? Style::Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin), QPen(c2, w2, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin))
					  : Style::Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin));
				}
				break;

			case 0x01:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3) && readColor(file, hdl, c4)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, c2.rgb());
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Style::Line(img);
				} else {
					if (!(file->readByte(hdl, w1) && file->readByte(hdl, w2)))
						return false;

					_lines[type] = (w2 > w1)
					  ? Style::Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin), QPen(c2, w2, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin))
					  : Style::Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin));
				}
				break;

			case 0x03:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, qRgba(255, 255, 255, 0));
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Style::Line(img);
				} else {
					if (!(file->readByte(hdl, w1) && file->readByte(hdl, w2)))
						return false;

					_lines[type] = Style::Line(QPen(c1, w1, Qt::SolidLine,
					  Qt::RoundCap, Qt::RoundJoin));
				}
				break;

			case 0x05:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, c2.rgb());
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Style::Line(img);
				} else {
					if (!(file->readByte(hdl, w1) && file->readByte(hdl, w2)))
						return false;

					_lines[type] = (w2 > w1)
					  ? Style::Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin), QPen(c2, w2, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin))
					  : Style::Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin));
				}
				break;

			case 0x06:
				if (!readColor(file, hdl, c1))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, qRgba(255, 255, 255, 0));
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Style::Line(img);
				} else {
					if (!file->readByte(hdl, w1))
						return false;

					_lines[type] = Style::Line(QPen(c1, w1, Qt::SolidLine,
					  Qt::RoundCap, Qt::RoundJoin));
				}
				break;

			case 0x07:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, qRgba(255,255,255,0));
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Style::Line(img);
				} else {
					if (!file->readByte(hdl, w1))
						return false;

					_lines[type] = Style::Line(QPen(c1, w1, Qt::SolidLine,
					  Qt::RoundCap, Qt::RoundJoin));
				}
				break;

			default:
				return false;
		}

		if (isContourLine(type)) {
			Line &l = _lines[type];
			l.setTextColor(l.foreground().color());
			l.setTextFontSize(Small);
		}

		if (localization && !skipLocalization(file, hdl))
			return false;

		if (textColor) {
			quint8 labelFlags;
			if (!file->readByte(hdl, labelFlags))
				return false;
			if (labelFlags & 0x08) {
				if (!readColor(file, hdl, c1))
					return false;
				_lines[type].setTextColor(c1);
			}
			_lines[type].setTextFontSize((FontSize)(labelFlags & 0x07));
		}
	}

	return true;
}

static int colors2bpp(quint8 colors, quint8 flags)
{
	switch (flags) {
		case 0x00:
			if (colors < 3)
				return colors;
			else if (colors == 3)
				return 2;
			else if (colors < 16)
				return 4;
			else
				return 8;
		case 0x10:
			if (colors == 0)
				return 1;
			else if (colors < 3)
				return 2;
			else if (colors < 15)
				return 4;
			else
				return 8;
		case 0x20:
			if (colors == 0)
				return -1;
			else if (colors < 3)
				return colors;
			else if (colors < 4)
				return 2;
			else if (colors < 16)
				return 4;
			else
				return 8;
		default:
			return -1;
	}
}

static bool readColorTable(SubFile *file, SubFile::Handle &hdl, QImage& img,
  int colors, int bpp, bool transparent)
{
	img.setColorCount(colors);

	if (transparent) {
		quint8 byte;
		quint32 bits = 0, reg = 0, mask = 0x000000FF;

		for (int i = 0; i < colors; i++) {
			while (bits < 28) {
				if (!file->readByte(hdl, byte))
					return false;

				mask = 0x000000FF << bits;
				reg  = reg & (~mask);
				reg  = reg | (byte << bits);
				bits += 8;
			}

			img.setColor(i, qRgba((reg >> 16) & 0x0FF, (reg >> 8) & 0x0FF,
			  reg & 0x0FF, ~((reg >> 24) & 0x0F) << 4));

			reg = reg >> 28;
			bits -= 28;
		}
		for (int i = colors; i < 1<<bpp; i++)
			img.setColor(i, qRgba(0, 0, 0, 0));
	} else {
		QColor color;

		for (int i = 0; i < colors; i++) {
			if (!readColor(file, hdl, color))
				return false;
			img.setColor(i, color.rgb());
		}
		for (int i = colors; i < 1<<bpp; i++)
			img.setColor(i, qRgba(0, 0, 0, 0));
	}

	return true;
}

bool Style::parsePoints(SubFile *file, SubFile::Handle &hdl,
  const Section &section)
{
	for (quint32 i = 0; i < section.arraySize / section.arrayItemSize; i++) {
		if (!file->seek(hdl, section.arrayOffset + i * section.arrayItemSize))
			return false;
		ItemInfo info;
		if (!itemInfo(file, hdl, section, info))
			return false;
		quint32 type = info.extended
		  ? 0x10000 | (info.type << 8) | info.subtype
		  : (info.type << 8) | info.subtype;

		quint8 t8_1, width, height, numColors, imgType;
		if (!(file->seek(hdl, section.offset + info.offset)
		  && file->readByte(hdl, t8_1) && file->readByte(hdl, width)
		  && file->readByte(hdl, height) && file->readByte(hdl, numColors)
		  && file->readByte(hdl, imgType)))
			return false;

		bool localization = t8_1 & 0x04;
		bool textColor = t8_1 & 0x08;

		int bpp = colors2bpp(numColors, imgType);
		if (bpp < 0)
			continue;
		QImage img(width, height, QImage::Format_Indexed8);
		if (!readColorTable(file, hdl, img, numColors, bpp, imgType == 0x20))
			return false;
		if (!readBitmap(file, hdl, img, bpp))
			return false;
		_points[type] = Point(img);

		if (t8_1 & 0x02) {
			if (!(file->readByte(hdl, numColors)
			  && file->readByte(hdl, imgType)))
				return false;
			if ((bpp = colors2bpp(numColors, imgType)) < 0)
				continue;
			if (!readColorTable(file, hdl, img, numColors, bpp, imgType == 0x20))
				return false;
			if (!readBitmap(file, hdl, img, bpp))
				return false;
		}

		if (localization && !skipLocalization(file, hdl))
			return false;

		if (textColor) {
			quint8 labelFlags;
			QColor color;
			if (!file->readByte(hdl, labelFlags))
				return false;
			if (labelFlags & 0x08) {
				if (!readColor(file, hdl, color))
					return false;
				_points[type].setTextColor(color);
			}
			_points[type].setTextFontSize((FontSize)(labelFlags & 0x07));
		}
	}

	return true;
}

bool Style::parseDrawOrder(SubFile *file, SubFile::Handle &hdl,
  const Section &order)
{
	QList<quint32> drawOrder;

	if (!file->seek(hdl, order.arrayOffset))
		return false;

	for (quint32 i = 0; i < order.arraySize / order.arrayItemSize; i++) {
		quint8 type;
		quint32 subtype;

		if (!(file->readByte(hdl, type) && file->readUInt32(hdl, subtype)))
			return false;

		if (!subtype)
			drawOrder.append(((quint32)type) << 8);
		else {
			for (int j = 0; j < 32; j++) {
				quint32 mask = 1 << j;
				if (subtype & mask)
					drawOrder.append(0x010000 | (((quint32)type) << 8) | j);
			}
		}
	}

	_drawOrder = drawOrder;

	return true;
}

bool Style::parseTYPFile(SubFile *file)
{
	SubFile::Handle hdl;
	Section points, lines, polygons, order;
	quint16 tmp16, codepage;

	if (!(file->seek(hdl, 0x15) && file->readUInt16(hdl, codepage)
	  && file->readUInt32(hdl, points.offset)
	  && file->readUInt32(hdl, points.size)
	  && file->readUInt32(hdl, lines.offset)
	  && file->readUInt32(hdl, lines.size)
	  && file->readUInt32(hdl, polygons.offset)
	  && file->readUInt32(hdl, polygons.size)))
		return false;

	if (!(file->readUInt16(hdl, tmp16) && file->readUInt16(hdl, tmp16)))
		return false;

	if (!(file->readUInt32(hdl, points.arrayOffset)
	  && file->readUInt16(hdl, points.arrayItemSize)
	  && file->readUInt32(hdl, points.arraySize)
	  && file->readUInt32(hdl, lines.arrayOffset)
	  && file->readUInt16(hdl, lines.arrayItemSize)
	  && file->readUInt32(hdl, lines.arraySize)
	  && file->readUInt32(hdl, polygons.arrayOffset)
	  && file->readUInt16(hdl, polygons.arrayItemSize)
	  && file->readUInt32(hdl, polygons.arraySize)
	  && file->readUInt32(hdl, order.arrayOffset)
	  && file->readUInt16(hdl, order.arrayItemSize)
	  && file->readUInt32(hdl, order.arraySize)))
		return false;

	if (!(parsePoints(file, hdl, points) && parseLines(file, hdl, lines)
	  && parsePolygons(file, hdl, polygons)
	  && parseDrawOrder(file, hdl, order))) {
		qWarning("%s: Invalid TYP file, using default style",
		  qPrintable(file->imgName()));
		return false;
	}

	return true;
}

Style::Style(SubFile *typ)
{
	defaultLineStyle();
	defaultPolygonStyle();

	if (typ)
		parseTYPFile(typ);
}

const Style::Line &Style::line(quint32 type) const
{
	static Line null;

	QMap<quint32, Line>::const_iterator it = _lines.find(type);
	return (it == _lines.constEnd()) ? null : *it;
}

const Style::Polygon &Style::polygon(quint32 type) const
{
	static Polygon null;

	QMap<quint32, Polygon>::const_iterator it = _polygons.find(type);
	return (it == _polygons.constEnd()) ? null : *it;
}

const Style::Point &Style::point(quint32 type) const
{
	static Point null;

	QMap<quint16, Point>::const_iterator it = _points.find(type);
	return (it == _points.constEnd()) ? null : *it;
}

bool Style::isContourLine(quint32 type)
{
	return (type == TYPE(0x20) || type == TYPE(0x21) || type == TYPE(0x22)
	  || type == TYPE(23) || type == TYPE(24) || type == TYPE(25));
}

#ifndef QT_NO_DEBUG
static QString penColor(const QPen &pen)
{
	return (pen == Qt::NoPen) ? "None" : pen.color().name();
}

static QString brushColor(const QBrush &brush)
{
	return (brush == Qt::NoBrush) ? "None" : brush.color().name();
}

QDebug operator<<(QDebug dbg, const Style::Polygon &polygon)
{
	dbg.nospace() << "Polygon(" << brushColor(polygon.brush()) << ", "
	  << penColor(polygon.pen()) << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const Style::Line &line)
{
	dbg.nospace() << "Line(" << penColor(line.foreground()) << ", "
	  << penColor(line.background()) << ", " << !line.img().isNull() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
