#ifndef IMAGETHUMBNAIL_H
#define IMAGETHUMBNAIL_H

#include "thumbnail.h"

class ImageThumbnail : public Thumbnail
{
public:
	ImageThumbnail(const QString &path, int limit, QWidget *parent = 0);
};

#endif // IMAGETHUMBNAIL_H
