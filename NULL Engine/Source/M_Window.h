#ifndef __MODULEWINDOW_H__
#define __MODULEWINDOW_H__

#include "Module.h"

class ParsonNode;

class MISSCLICK_API M_Window : public Module
{
public:
	M_Window(bool isActive = true);
	virtual ~M_Window();

	bool Init(ParsonNode& config) override;
	bool CleanUp() override;

	bool LoadConfiguration(ParsonNode& root) override;
	bool SaveConfiguration(ParsonNode& root) const override;
	
public:
	SDL_Window* GetWindow() const;												// 
	void		SetTitle(const char* title);									// 

	const char* GetIcon() const;												// 
	void		SetIcon(const char* file);										// 
	void		SetDefaultIcon();												// 

	uint		GetWidth() const;												// 
	uint		GetHeight() const;												// 
	void		GetMinMaxSize(uint& minWidth, uint& minHeight, uint& maxWidth, uint& maxHeight) const;	// 
	float		GetBrightness() const;											// 
	int			GetRefreshRate() const;											// 

	void		SetWidth(uint width);											// 
	void		SetHeight(uint height);											// 
	void		SetSize(uint width, uint height);								// 
	void		SetBrightness(float brightness);								// 

	bool		IsMaximized() const;											// 
	bool		IsFullscreen() const;											// 
	bool		IsResizable() const;											// 
	bool		IsBorderless() const;											// 
	bool		IsFullscreenDesktop() const;									// 

	void		SetMaximized(bool setTo);										// 
	void		SetFullscreen(bool setTo);										// 
	void		SetResizable(bool setTo);										// 
	void		SetBorderless(bool setTo);										// 
	void		SetFullscreenDesktop(bool setTo);								// 

	void		RecalculateWindowSize();

private:
	SDL_Window*		window;														// The window we'll be rendering to
	SDL_Surface*	screenSurface;												// The surface contained by the window

	std::string		iconFile;

	uint			screenWidth;												// Width of the window in pixels.
	uint			screenHeight;												// Height of the window in pixels.

	bool			isMaximized;												// 
	bool			isFullscreen;												// 
	bool			isResizable;												// 
	bool			isBorderless;												// 
	bool			isFullscreenDesktop;										// 
};

#endif // __MODULEWINDOW_H__