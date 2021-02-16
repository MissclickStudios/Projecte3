#ifndef __IMPORT_SETTINGS_H__
#define __IMPORT_SETTINGS_H__

class ParsonNode;

enum class AXIS_ORIENTATION
{
	X_UP,
	Y_UP,
	Z_UP
};

class ImportSettings													// TMP. Brought from Importer.h
{
public:
	ImportSettings();
	 virtual ~ImportSettings();

	virtual bool Save(ParsonNode& settings) const;
	virtual bool Load(const ParsonNode& settings);

public:
	float				global_scale;
	AXIS_ORIENTATION	axis;
	bool				ignore_cameras;
	bool				ignore_lights;

	int					compression;
	bool				flip_X;
	bool				flip_Y;
	int					wrapping;
	int					filter;
	bool				generate_mipmaps;
	bool				anisotropy;

private:

};

#endif // !__IMPORT_SETTINGS_H__
