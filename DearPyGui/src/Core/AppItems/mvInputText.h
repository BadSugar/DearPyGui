#pragma once

#include "Core/AppItems/mvTypeBases.h"
#include "Core/mvDataStorage.h"
#include <misc/cpp/imgui_stdlib.h>
#include <utility>

//-----------------------------------------------------------------------------
// Widget Index
//
//     * mvInputText
//
//-----------------------------------------------------------------------------

namespace Marvel {

	class mvInputText : public mvStringItemBase
	{

	public:

		MV_APPITEM_TYPE(mvAppItemType::InputText, "add_input_text")

		mvInputText(const std::string& name, const std::string& default_value)
			: mvStringItemBase(name, default_value)
		{
		}

		void draw() override
		{
			pushColorStyles();

			if (m_multiline)
				m_hint = "";

			if (m_hint.empty())
			{
				if(m_multiline)
				{
					if (ImGui::InputTextMultiline(m_label.c_str(), &m_value, ImVec2((float)m_width, (float)m_height), m_flags))
					{
						if (!m_dataSource.empty())
							mvDataStorage::AddData(m_dataSource, getPyValue());

						mvApp::GetApp()->runCallback(m_callback, m_name, m_callbackData);

						// Context Menu
						if (!getPopup().empty())
							ImGui::OpenPopup(getPopup().c_str());
					}
				}
				else
				{
					if (ImGui::InputText(m_label.c_str(), &m_value, m_flags))
					{
						if (!m_dataSource.empty())
							mvDataStorage::AddData(m_dataSource, getPyValue());

						mvApp::GetApp()->runCallback(m_callback, m_name);

						// Context Menu
						if (!getPopup().empty())
							ImGui::OpenPopup(getPopup().c_str());
					}
				}
			}

			else
			{
				if (ImGui::InputTextWithHint(m_label.c_str(), m_hint.c_str(), &m_value, m_flags))
				{
					if (!m_dataSource.empty())
						mvDataStorage::AddData(m_dataSource, getPyValue());

					mvApp::GetApp()->runCallback(m_callback, m_name, m_callbackData);

					// Context Menu
					if (!getPopup().empty())
						ImGui::OpenPopup(getPopup().c_str());
				}
			}

			// Regular Tooltip (simple)
			if (!getTip().empty() && ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", getTip().c_str());

			popColorStyles();
		}

		void setExtraConfigDict(PyObject* dict) override
		{
			if (dict == nullptr)
				return;
			mvGlobalIntepreterLock gil;
			if (PyObject* item = PyDict_GetItemString(dict, "hint")) m_hint = ToString(item);
			if (PyObject* item = PyDict_GetItemString(dict, "multiline")) m_multiline = ToBool(item);

			// helper for bit flipping
			auto flagop = [dict](const char* keyword, int flag, int& flags)
			{
				if (PyObject* item = PyDict_GetItemString(dict, keyword)) ToBool(item) ? flags |= flag : flags &= ~flag;
			};

			// flags
			flagop("no_spaces",   ImGuiInputTextFlags_CharsNoBlank,     m_flags);
			flagop("uppercase",   ImGuiInputTextFlags_CharsUppercase,   m_flags);
			flagop("decimal",     ImGuiInputTextFlags_CharsDecimal,     m_flags);
			flagop("hexadecimal", ImGuiInputTextFlags_CharsHexadecimal, m_flags);
			flagop("readonly",    ImGuiInputTextFlags_ReadOnly,         m_flags);
			flagop("password",    ImGuiInputTextFlags_Password,         m_flags);
			flagop("on_enter",    ImGuiInputTextFlags_EnterReturnsTrue, m_flags);
		}

		void getExtraConfigDict(PyObject* dict) override
		{
			if (dict == nullptr)
				return;
			mvGlobalIntepreterLock gil;
			PyDict_SetItemString(dict, "hint", ToPyString(m_hint));
			PyDict_SetItemString(dict, "multline", ToPyBool(m_multiline));

			// helper to check and set bit
			auto checkbitset = [dict](const char* keyword, int flag, const int& flags)
			{
				PyDict_SetItemString(dict, keyword, ToPyBool(flags & flag));
			};

			// window flags
			checkbitset("no_spaces",   ImGuiInputTextFlags_CharsNoBlank,     m_flags);
			checkbitset("uppercase",   ImGuiInputTextFlags_CharsUppercase,   m_flags);
			checkbitset("decimal",     ImGuiInputTextFlags_CharsDecimal,     m_flags);
			checkbitset("hexadecimal", ImGuiInputTextFlags_CharsHexadecimal, m_flags);
			checkbitset("readonly",    ImGuiInputTextFlags_ReadOnly,         m_flags);
			checkbitset("password",    ImGuiInputTextFlags_Password,         m_flags);
			checkbitset("on_enter",    ImGuiInputTextFlags_EnterReturnsTrue, m_flags);
		}

	private:

		std::string         m_hint;
		bool                m_multiline;
		ImGuiInputTextFlags m_flags = 0;

	};

}