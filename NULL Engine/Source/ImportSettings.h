#ifndef __IMPORT_SETTINGS_H__
#define __IMPORT_SETTINGS_H__

class ParsonNode;

enum class AxisOrientation
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
	float				globalScale;
	AxisOrientation		axis;
	bool				ignoreCameras;
	bool				ignoreLights;

	int					compression;
	bool				flipX;
	bool				flipY;
	int					wrapping;
	int					filter;
	bool				generateMipmaps;
	bool				anisotropy;

private:

};

#endif // !__IMPORT_SETTINGS_H__
