#ifndef ZIP_H
#define ZIP_H

#include <QByteArray>
#include <QHash>
#include <QStringList>

class QIODevice;

class Zip
{
public:
	Zip(const QString &path);
	Zip(QIODevice *device);
	~Zip();

	bool isValid() const {return _valid;}
	QStringList files() const {return _fileHeaders.keys();}
	QByteArray file(const QString &fileName) const;

	static bool isZIP(QIODevice *device);

private:
	struct EndOfDirectory
	{
		uchar signature[4];
		uchar this_disk[2];
		uchar start_of_directory_disk[2];
		uchar num_dir_entries_this_disk[2];
		uchar num_dir_entries[2];
		uchar directory_size[4];
		uchar dir_start_offset[4];
		uchar comment_length[2];
	};

	struct CentralFileHeader
	{
		uchar signature[4];
		uchar version_made[2];
		uchar version_needed[2];
		uchar general_purpose_bits[2];
		uchar compression_method[2];
		uchar last_mod_file[4];
		uchar crc_32[4];
		uchar compressed_size[4];
		uchar uncompressed_size[4];
		uchar file_name_length[2];
		uchar extra_field_length[2];
		uchar file_comment_length[2];
		uchar disk_start[2];
		uchar internal_file_attributes[2];
		uchar external_file_attributes[4];
		uchar offset_local_header[4];
	};

	struct LocalFileHeader
	{
		uchar signature[4];
		uchar version_needed[2];
		uchar general_purpose_bits[2];
		uchar compression_method[2];
		uchar last_mod_file[4];
		uchar crc_32[4];
		uchar compressed_size[4];
		uchar uncompressed_size[4];
		uchar file_name_length[2];
		uchar extra_field_length[2];
	};

	bool readHeaders();

	QIODevice *_device;
	bool _deleteDevice;
	bool _valid;
	QHash<QString, CentralFileHeader> _fileHeaders;
};

#endif // ZIP_H
