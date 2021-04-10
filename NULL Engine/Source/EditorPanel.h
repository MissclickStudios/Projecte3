#ifndef __EDITOR_PANEL_H__
#define __EDITOR_PANEL_H__

#include <string>
#include <vector>

#include "ImGui.h"
#include "Log.h"

class EditorPanel
{
public:
	EditorPanel();
	EditorPanel(const char* name, bool isActive = true);
	virtual ~EditorPanel();

	virtual bool	Draw	(ImGuiIO& io);
	virtual bool	CleanUp	();

public:
	void			Enable			();
	void			Disable			();

	const char*		GetName			() const;
	bool			IsActive		() const;
	bool			IsHovered		() const;
	bool			IsClicked		() const;

	void			SetIsHovered	();													// Trying out having a Set() function without any parameters. Kinda dirty.
	void			SetIsClicked	(const bool& setTo);

	void			HelpMarker		(const char* description);

private:
	const char*		name;
	bool			isActive;
	bool			isHovered;
	bool			isClicked;
};

#endif // !__E_PANEL_H__