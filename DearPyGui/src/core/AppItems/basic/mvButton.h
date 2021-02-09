#pragma once

#include "mvTypeBases.h"

namespace Marvel {

	struct mvButtonConfig : public mvAppItemConfig
	{
		ImGuiDir direction = ImGuiDir_Up;
		bool small_button = false;
		bool arrow = false;
	};

#ifdef MV_CPP
	void add_button(const char* name, const mvButtonConfig& config = {});
	void add_button(const char* name, mvCallable callable);
#else
	PyObject* add_button(PyObject* self, PyObject* args, PyObject* kwargs);
#endif
	

	class mvButton : public mvAppItem
	{

		MV_APPITEM_TYPE(mvAppItemType::Button, mvButton, "add_button")

		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeCol_Button_Text			,  0L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeCol_Button_Bg			, 21L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeCol_Button_Hovered		, 22L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeCol_Button_Active		, 23L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeCol_Button_Border		,  5L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeCol_Button_BorderShadow	,  6L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeStyle_Button_Rounding	, 11L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeStyle_Button_BorderSize	, 12L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeStyle_Button_TextAlignX	, 22L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeStyle_Button_TextAlignY	, 22L, 1L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeStyle_Button_PaddingX	, 10L, 0L);
		MV_CREATE_THEME_CONSTANT(mvAppItemType::Button, mvThemeStyle_Button_PaddingY	, 10L, 1L);

		MV_START_COLOR_CONSTANTS
			MV_CREATE_CONSTANT_PAIR(mvThemeCol_Button_Text),
			MV_CREATE_CONSTANT_PAIR(mvThemeCol_Button_Bg),
			MV_CREATE_CONSTANT_PAIR(mvThemeCol_Button_Hovered),
			MV_CREATE_CONSTANT_PAIR(mvThemeCol_Button_Active),
			MV_CREATE_CONSTANT_PAIR(mvThemeCol_Button_Border),
			MV_CREATE_CONSTANT_PAIR(mvThemeCol_Button_BorderShadow),
		MV_END_COLOR_CONSTANTS

		MV_START_STYLE_CONSTANTS
			MV_CREATE_CONSTANT_PAIR(mvThemeStyle_Button_Rounding),
			MV_CREATE_CONSTANT_PAIR(mvThemeStyle_Button_BorderSize),
			MV_CREATE_CONSTANT_PAIR(mvThemeStyle_Button_TextAlignX),
			MV_CREATE_CONSTANT_PAIR(mvThemeStyle_Button_TextAlignY),
			MV_CREATE_CONSTANT_PAIR(mvThemeStyle_Button_PaddingX),
			MV_CREATE_CONSTANT_PAIR(mvThemeStyle_Button_PaddingY),
		MV_END_STYLE_CONSTANTS


	public:

		static void InsertParser(std::map<std::string, mvPythonParser>* parsers);

	public:

		mvButton(const std::string& name);
		mvButton(const std::string& name, const mvButtonConfig& config);

		void draw() override;

#ifndef MV_CPP
		void setExtraConfigDict(PyObject* dict) override;
		void getExtraConfigDict(PyObject* dict) override;
#endif // !MV_CPP

		// cpp interface
		void updateConfig(mvAppItemConfig* config) override;
		mvAppItemConfig* getConfig() override;

	private:

		mvButtonConfig m_config;

	};

}
