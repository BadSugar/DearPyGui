#pragma once

#include "mvApp.h"
#include "Core/AppItems/mvTypeBases.h"
#include <implot.h>
#include <implot_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include "mvPythonTranslator.h"


//-----------------------------------------------------------------------------
// Widget Index
//
//     * mvDatePicker
//
//-----------------------------------------------------------------------------

namespace Marvel {

	class mvDatePicker : public mvTimeItemBase
	{

	public:

		MV_APPITEM_TYPE(mvAppItemType::DatePicker, "add_date_picker")

			mvDatePicker(const std::string& name, tm default_value)
			: mvTimeItemBase(name, default_value)
		{
		}

		void draw() override
		{
			pushColorStyles();

			if (ImPlot::ShowDatePicker(m_name.c_str(), &m_level, &m_imvalue, &m_imvalue))
			{
				ImPlot::GetGmtTime(m_imvalue, &m_value);
				mvApp::GetApp()->runCallback(m_callback, m_name, m_callbackData);
			}


			popColorStyles();
		}

		void setExtraConfigDict(PyObject* dict) override
		{
			if (dict == nullptr)
				return;
			mvGlobalIntepreterLock gil;
			if (PyObject* item = PyDict_GetItemString(dict, "level")) m_level = ToInt(item);
		}

		void getExtraConfigDict(PyObject* dict) override
		{
			if (dict == nullptr)
				return;
			mvGlobalIntepreterLock gil;
			PyDict_SetItemString(dict, "level", ToPyInt(m_level));
		}

	private:

		int m_level = 0;


	};

}