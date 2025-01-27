#include "mvDrag.h"
#include <utility>
#include "mvContext.h"
#include "dearpygui.h"
#include <string>
#include "mvItemRegistry.h"
#include "mvPythonExceptions.h"
#include "AppItems/fonts/mvFont.h"
#include "AppItems/themes/mvTheme.h"
#include "AppItems/containers/mvDragPayload.h"
#include "mvPyObject.h"
#include "AppItems/widget_handlers/mvItemHandlerRegistry.h"

namespace Marvel {

    mvDragFloatMulti::mvDragFloatMulti(mvUUID uuid)
        : mvAppItem(uuid)
    {
    }

    void mvDragFloatMulti::applySpecificTemplate(mvAppItem* item)
    {
        auto titem = static_cast<mvDragFloatMulti*>(item);
        if (config.source != 0) _value = titem->_value;
        _disabled_value[0] = titem->_disabled_value[0];
        _disabled_value[1] = titem->_disabled_value[1];
        _disabled_value[2] = titem->_disabled_value[2];
        _disabled_value[3] = titem->_disabled_value[3];
       _speed = titem->_speed;
       _min = titem->_min;
       _max = titem->_max;
       _format = titem->_format;
       _flags = titem->_flags;
       _stor_flags = titem->_stor_flags;
       _size = titem->_size;
    }

    PyObject* mvDragFloatMulti::getPyValue()
    {
        return ToPyFloatList(_value->data(), 4);
    }

    void mvDragFloatMulti::setPyValue(PyObject* value)
    {
        std::vector<float> temp = ToFloatVect(value);
        while (temp.size() < 4)
            temp.push_back(0.0f);
        std::array<float, 4> temp_array;
        for (size_t i = 0; i < temp_array.size(); i++)
            temp_array[i] = temp[i];
        if (_value)
            *_value = temp_array;
        else
            _value = std::make_shared<std::array<float, 4>>(temp_array);
    }

    void mvDragFloatMulti::setDataSource(mvUUID dataSource)
    {
        if (dataSource == config.source) return;
        config.source = dataSource;

        mvAppItem* item = GetItem((*GContext->itemRegistry), dataSource);
        if (!item)
        {
            mvThrowPythonError(mvErrorCode::mvSourceNotFound, "set_value",
                "Source item not found: " + std::to_string(dataSource), this);
            return;
        }
        if (GetEntityValueType(item->type) != GetEntityValueType(type))
        {
            mvThrowPythonError(mvErrorCode::mvSourceNotCompatible, "set_value",
                "Values types do not match: " + std::to_string(dataSource), this);
            return;
        }
        _value = *static_cast<std::shared_ptr<std::array<float, 4>>*>(item->getValue());
    }

    void mvDragFloatMulti::draw(ImDrawList* drawlist, float x, float y)
    {

        //-----------------------------------------------------------------------------
        // pre draw
        //-----------------------------------------------------------------------------

        // show/hide
        if (!config.show)
            return;

        // focusing
        if (info.focusNextFrame)
        {
            ImGui::SetKeyboardFocusHere();
            info.focusNextFrame = false;
        }

        // cache old cursor position
        ImVec2 previousCursorPos = ImGui::GetCursorPos();

        // set cursor position if user set
        if (info.dirtyPos)
            ImGui::SetCursorPos(state.pos);

        // update widget's position state
        state.pos = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };

        // set item width
        if (config.width != 0)
            ImGui::SetNextItemWidth((float)config.width);

        // set indent
        if (config.indent > 0.0f)
            ImGui::Indent(config.indent);

        // push font if a font object is attached
        if (font)
        {
            ImFont* fontptr = static_cast<mvFont*>(font.get())->getFontPtr();
            ImGui::PushFont(fontptr);
        }

        // themes
        apply_local_theming(this);

        //-----------------------------------------------------------------------------
        // draw
        //-----------------------------------------------------------------------------
        {
            ScopedID id(uuid);

            bool activated = false;

            if(!config.enabled) std::copy(_value->data(), _value->data() + 2, _disabled_value);

            switch (_size)
            {
            case 2:
                activated = ImGui::DragFloat2(info.internalLabel.c_str(), config.enabled ? _value->data() : &_disabled_value[0], _speed, _min, _max, _format.c_str(), _flags);
                break;
            case 3:
                activated = ImGui::DragFloat3(info.internalLabel.c_str(), config.enabled ? _value->data() : &_disabled_value[0], _speed, _min, _max, _format.c_str(), _flags);
                break;
            case 4:
                activated = ImGui::DragFloat4(info.internalLabel.c_str(), config.enabled ? _value->data() : &_disabled_value[0], _speed, _min, _max, _format.c_str(), _flags);
                break;
            default:
                break;
            }

            if (activated)
            {
                auto value = *_value;
                if(config.alias.empty())
                    mvSubmitCallback([=]() {
                    mvAddCallback(getCallback(false), uuid, ToPyFloatList(value.data(), (int)value.size()), config.user_data);
                        });
                else
                    mvSubmitCallback([=]() {
                    mvAddCallback(getCallback(false), config.alias, ToPyFloatList(value.data(), (int)value.size()), config.user_data);
                        });
            }
        }
        //-----------------------------------------------------------------------------
        // update state
        //-----------------------------------------------------------------------------
        UpdateAppItemState(state);

        //-----------------------------------------------------------------------------
        // post draw
        //-----------------------------------------------------------------------------

        // set cursor position to cached position
        if (info.dirtyPos)
            ImGui::SetCursorPos(previousCursorPos);

        if (config.indent > 0.0f)
            ImGui::Unindent(config.indent);

        // pop font off stack
        if (font)
            ImGui::PopFont();

        // handle popping themes
        cleanup_local_theming(this);

        if (handlerRegistry)
            handlerRegistry->checkEvents(&state);

        // handle drag & drop if used
        apply_drag_drop(this);
    }

    void mvDragFloatMulti::handleSpecificKeywordArgs(PyObject* dict)
    {
        if (dict == nullptr)
            return;

        if (PyObject* item = PyDict_GetItemString(dict, "format")) _format = ToString(item);
        if (PyObject* item = PyDict_GetItemString(dict, "speed")) _speed = ToFloat(item);
        if (PyObject* item = PyDict_GetItemString(dict, "min_value")) _min = ToFloat(item);
        if (PyObject* item = PyDict_GetItemString(dict, "max_value")) _max = ToFloat(item);
        if (PyObject* item = PyDict_GetItemString(dict, "size")) _size = ToInt(item);

        // helper for bit flipping
        auto flagop = [dict](const char* keyword, int flag, int& flags)
        {
            if (PyObject* item = PyDict_GetItemString(dict, keyword)) ToBool(item) ? flags |= flag : flags &= ~flag;
        };

        // flags
        flagop("clamped", ImGuiSliderFlags_ClampOnInput, _flags);
        flagop("clamped", ImGuiSliderFlags_ClampOnInput, _stor_flags);
        flagop("no_input", ImGuiSliderFlags_NoInput, _flags);
        flagop("no_input", ImGuiSliderFlags_NoInput, _stor_flags);

        if (info.enabledLastFrame)
        {
            info.enabledLastFrame = false;
            _flags = _stor_flags;
        }

        if (info.disabledLastFrame)
        {
            info.disabledLastFrame = false;
            _stor_flags = _flags;
            _flags |= ImGuiSliderFlags_NoInput;
        }

    }

    void mvDragFloatMulti::getSpecificConfiguration(PyObject* dict)
    {
        if (dict == nullptr)
            return;

        mvPyObject py_format = ToPyString(_format);
        mvPyObject py_speed = ToPyFloat(_speed);
        mvPyObject py_min_value = ToPyFloat(_min);
        mvPyObject py_max_value = ToPyFloat(_max);
        mvPyObject py_size = ToPyInt(_size);

        PyDict_SetItemString(dict, "format", py_format);
        PyDict_SetItemString(dict, "speed", py_speed);
        PyDict_SetItemString(dict, "min_value", py_min_value);
        PyDict_SetItemString(dict, "max_value", py_max_value);
        PyDict_SetItemString(dict, "size", py_size);

        // helper to check and set bit
        auto checkbitset = [dict](const char* keyword, int flag, const int& flags)
        {
            mvPyObject py_result = ToPyBool(flags & flag);
            PyDict_SetItemString(dict, keyword, py_result);
        };

        // window flags
        checkbitset("clamped", ImGuiSliderFlags_ClampOnInput, _flags);
        checkbitset("no_input", ImGuiSliderFlags_NoInput, _flags);

    }

    mvDragIntMulti::mvDragIntMulti(mvUUID uuid)
        : mvAppItem(uuid)
    {
    }

    void mvDragIntMulti::applySpecificTemplate(mvAppItem* item)
    {
        auto titem = static_cast<mvDragIntMulti*>(item);
        if (config.source != 0) _value = titem->_value;
        _disabled_value[0] = titem->_disabled_value[0];
        _disabled_value[1] = titem->_disabled_value[1];
        _disabled_value[2] = titem->_disabled_value[2];
        _disabled_value[3] = titem->_disabled_value[3];
        _speed = titem->_speed;
        _min = titem->_min;
        _max = titem->_max;
        _format = titem->_format;
        _flags = titem->_flags;
        _stor_flags = titem->_stor_flags;
        _size = titem->_size;
    }

    PyObject* mvDragIntMulti::getPyValue()
    {
        return ToPyIntList(_value->data(), 4);
    }

    void mvDragIntMulti::setPyValue(PyObject* value)
    {
        std::vector<int> temp = ToIntVect(value);
        while (temp.size() < 4)
            temp.push_back(0);
        std::array<int, 4> temp_array;
        for (size_t i = 0; i < temp_array.size(); i++)
            temp_array[i] = temp[i];
        if (_value)
            *_value = temp_array;
        else
            _value = std::make_shared<std::array<int, 4>>(temp_array);
    }

    void mvDragIntMulti::setDataSource(mvUUID dataSource)
    {
        if (dataSource == config.source) return;
        config.source = dataSource;

        mvAppItem* item = GetItem((*GContext->itemRegistry), dataSource);
        if (!item)
        {
            mvThrowPythonError(mvErrorCode::mvSourceNotFound, "set_value",
                "Source item not found: " + std::to_string(dataSource), this);
            return;
        }
        if (GetEntityValueType(item->type) != GetEntityValueType(type))
        {
            mvThrowPythonError(mvErrorCode::mvSourceNotCompatible, "set_value",
                "Values types do not match: " + std::to_string(dataSource), this);
            return;
        }
        _value = *static_cast<std::shared_ptr<std::array<int, 4>>*>(item->getValue());
    }

    void mvDragIntMulti::draw(ImDrawList* drawlist, float x, float y)
    {

        //-----------------------------------------------------------------------------
        // pre draw
        //-----------------------------------------------------------------------------

        // show/hide
        if (!config.show)
            return;

        // focusing
        if (info.focusNextFrame)
        {
            ImGui::SetKeyboardFocusHere();
            info.focusNextFrame = false;
        }

        // cache old cursor position
        ImVec2 previousCursorPos = ImGui::GetCursorPos();

        // set cursor position if user set
        if (info.dirtyPos)
            ImGui::SetCursorPos(state.pos);

        // update widget's position state
        state.pos = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };

        // set item width
        if (config.width != 0)
            ImGui::SetNextItemWidth((float)config.width);

        // set indent
        if (config.indent > 0.0f)
            ImGui::Indent(config.indent);

        // push font if a font object is attached
        if (font)
        {
            ImFont* fontptr = static_cast<mvFont*>(font.get())->getFontPtr();
            ImGui::PushFont(fontptr);
        }

        // themes
        apply_local_theming(this);


        //-----------------------------------------------------------------------------
        // draw
        //-----------------------------------------------------------------------------
        {

            ScopedID id(uuid);

            bool activated = false;

            if(!config.enabled) std::copy(_value->data(), _value->data() + 2, _disabled_value);

            switch (_size)
            {
            case 2:
                activated = ImGui::DragInt2(info.internalLabel.c_str(), config.enabled ? _value->data() : &_disabled_value[0], _speed, _min, _max, _format.c_str(), _flags);
                break;
            case 3:
                activated = ImGui::DragInt3(info.internalLabel.c_str(), config.enabled ? _value->data() : &_disabled_value[0], _speed, _min, _max, _format.c_str(), _flags);
                break;
            case 4:
                activated = ImGui::DragInt4(info.internalLabel.c_str(), config.enabled ? _value->data() : &_disabled_value[0], _speed, _min, _max, _format.c_str(), _flags);
                break;
            default:
                break;
            }

            if (activated)
            {
                auto value = *_value;
                if(config.alias.empty())
                    mvSubmitCallback([=]() {
                    mvAddCallback(getCallback(false), uuid, ToPyIntList(value.data(), (int)value.size()), config.user_data);
                        });
                else
                    mvSubmitCallback([=]() {
                    mvAddCallback(getCallback(false), config.alias, ToPyIntList(value.data(), (int)value.size()), config.user_data);
                        });
            }
        }

        //-----------------------------------------------------------------------------
        // update state
        //-----------------------------------------------------------------------------
        UpdateAppItemState(state);

        //-----------------------------------------------------------------------------
        // post draw
        //-----------------------------------------------------------------------------

        // set cursor position to cached position
        if (info.dirtyPos)
            ImGui::SetCursorPos(previousCursorPos);

        if (config.indent > 0.0f)
            ImGui::Unindent(config.indent);

        // pop font off stack
        if (font)
            ImGui::PopFont();

        // handle popping themes
        cleanup_local_theming(this);

        if (handlerRegistry)
            handlerRegistry->checkEvents(&state);

        // handle drag & drop if used
        apply_drag_drop(this);
    }

    void mvDragIntMulti::handleSpecificKeywordArgs(PyObject* dict)
    {
        if (dict == nullptr)
            return;

        if (PyObject* item = PyDict_GetItemString(dict, "format")) _format = ToString(item);
        if (PyObject* item = PyDict_GetItemString(dict, "speed")) _speed = ToFloat(item);
        if (PyObject* item = PyDict_GetItemString(dict, "min_value")) _min = ToInt(item);
        if (PyObject* item = PyDict_GetItemString(dict, "max_value")) _max = ToInt(item);
        if (PyObject* item = PyDict_GetItemString(dict, "size")) _size = ToInt(item);

        // helper for bit flipping
        auto flagop = [dict](const char* keyword, int flag, int& flags)
        {
            if (PyObject* item = PyDict_GetItemString(dict, keyword)) ToBool(item) ? flags |= flag : flags &= ~flag;
        };

        // flags
        flagop("clamped", ImGuiSliderFlags_ClampOnInput, _flags);
        flagop("clamped", ImGuiSliderFlags_ClampOnInput, _stor_flags);
        flagop("no_input", ImGuiSliderFlags_NoInput, _flags);
        flagop("no_input", ImGuiSliderFlags_NoInput, _stor_flags);

        if (info.enabledLastFrame)
        {
            info.enabledLastFrame = false;
            _flags = _stor_flags;
        }

        if (info.disabledLastFrame)
        {
            info.disabledLastFrame = false;
            _stor_flags = _flags;
            _flags |= ImGuiSliderFlags_NoInput;
        }

    }

    void mvDragIntMulti::getSpecificConfiguration(PyObject* dict)
    {
        if (dict == nullptr)
            return;

        mvPyObject py_format = ToPyString(_format);
        mvPyObject py_speed = ToPyFloat(_speed);
        mvPyObject py_min_value = ToPyInt(_min);
        mvPyObject py_max_value = ToPyInt(_max);
        mvPyObject py_size = ToPyInt(_size);

        PyDict_SetItemString(dict, "format", py_format);
        PyDict_SetItemString(dict, "speed", py_speed);
        PyDict_SetItemString(dict, "min_value", py_min_value);
        PyDict_SetItemString(dict, "max_value", py_max_value);
        PyDict_SetItemString(dict, "size", py_size);

        // helper to check and set bit
        auto checkbitset = [dict](const char* keyword, int flag, const int& flags)
        {
            mvPyObject py_result = ToPyBool(flags & flag);
            PyDict_SetItemString(dict, keyword, py_result);
        };

        // window flags
        checkbitset("clamped", ImGuiSliderFlags_ClampOnInput, _flags);
        checkbitset("no_input", ImGuiSliderFlags_NoInput, _flags);

    }

    mvDragFloat::mvDragFloat(mvUUID uuid)
        : mvAppItem(uuid)
    {
    }

    void mvDragFloat::applySpecificTemplate(mvAppItem* item)
    {
        auto titem = static_cast<mvDragFloat*>(item);
        if (config.source != 0) _value = titem->_value;
        _disabled_value = titem->_disabled_value;
        _speed = titem->_speed;
        _min = titem->_min;
        _max = titem->_max;
        _format = titem->_format;
        _flags = titem->_flags;
        _stor_flags = titem->_stor_flags;
    }

    PyObject* mvDragFloat::getPyValue()
    {
        return ToPyFloat(*_value);
    }

    void mvDragFloat::setPyValue(PyObject* value)
    {
        *_value = ToFloat(value);
    }

    void mvDragFloat::setDataSource(mvUUID dataSource)
    {
        if (dataSource == config.source) return;
        config.source = dataSource;

        mvAppItem* item = GetItem((*GContext->itemRegistry), dataSource);
        if (!item)
        {
            mvThrowPythonError(mvErrorCode::mvSourceNotFound, "set_value",
                "Source item not found: " + std::to_string(dataSource), this);
            return;
        }
        if (GetEntityValueType(item->type) != GetEntityValueType(type))
        {
            mvThrowPythonError(mvErrorCode::mvSourceNotCompatible, "set_value",
                "Values types do not match: " + std::to_string(dataSource), this);
            return;
        }
        _value = *static_cast<std::shared_ptr<float>*>(item->getValue());
    }

    void mvDragFloat::draw(ImDrawList* drawlist, float x, float y)
    {
        ScopedID id(uuid);


        //-----------------------------------------------------------------------------
        // pre draw
        //-----------------------------------------------------------------------------

        // show/hide
        if (!config.show)
            return;

        // focusing
        if (info.focusNextFrame)
        {
            ImGui::SetKeyboardFocusHere();
            info.focusNextFrame = false;
        }

        // cache old cursor position
        ImVec2 previousCursorPos = ImGui::GetCursorPos();

        // set cursor position if user set
        if (info.dirtyPos)
            ImGui::SetCursorPos(state.pos);

        // update widget's position state
        state.pos = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };

        // set item width
        if (config.width != 0)
            ImGui::SetNextItemWidth((float)config.width);

        // set indent
        if (config.indent > 0.0f)
            ImGui::Indent(config.indent);

        // push font if a font object is attached
        if (font)
        {
            ImFont* fontptr = static_cast<mvFont*>(font.get())->getFontPtr();
            ImGui::PushFont(fontptr);
        }

        // themes
        apply_local_theming(this);

        //-----------------------------------------------------------------------------
        // draw
        //-----------------------------------------------------------------------------
        {

            if(!config.enabled) _disabled_value = *_value;

            if (ImGui::DragFloat(info.internalLabel.c_str(), config.enabled ? _value.get() : &_disabled_value, _speed, _min, _max, _format.c_str(), _flags))
            {
                auto value = *_value;

                if(config.alias.empty())
                    mvSubmitCallback([=]() {
                    mvAddCallback(getCallback(false), uuid, ToPyFloat(value), config.user_data);
                        });
                else
                    mvSubmitCallback([=]() {
                    mvAddCallback(getCallback(false), config.alias, ToPyFloat(value), config.user_data);
                        });
            }
        }

        //-----------------------------------------------------------------------------
        // update state
        //-----------------------------------------------------------------------------
        UpdateAppItemState(state);

        //-----------------------------------------------------------------------------
        // post draw
        //-----------------------------------------------------------------------------

        // set cursor position to cached position
        if (info.dirtyPos)
            ImGui::SetCursorPos(previousCursorPos);

        if (config.indent > 0.0f)
            ImGui::Unindent(config.indent);

        // pop font off stack
        if (font)
            ImGui::PopFont();

        // handle popping themes
        cleanup_local_theming(this);

        if (handlerRegistry)
            handlerRegistry->checkEvents(&state);

        // handle drag & drop if used
        apply_drag_drop(this);
    }

    mvDragInt::mvDragInt(mvUUID uuid)
        : mvAppItem(uuid)
    {
    }

    void mvDragInt::applySpecificTemplate(mvAppItem* item)
    {
        auto titem = static_cast<mvDragInt*>(item);
        if (config.source != 0) _value = titem->_value;
        _disabled_value = titem->_disabled_value;
        _speed = titem->_speed;
        _min = titem->_min;
        _max = titem->_max;
        _format = titem->_format;
        _flags = titem->_flags;
        _stor_flags = titem->_stor_flags;
    }

    void mvDragInt::setDataSource(mvUUID dataSource)
    {
        if (dataSource == config.source) return;
        config.source = dataSource;

        mvAppItem* item = GetItem((*GContext->itemRegistry), dataSource);
        if (!item)
        {
            mvThrowPythonError(mvErrorCode::mvSourceNotFound, "set_value",
                "Source item not found: " + std::to_string(dataSource), this);
            return;
        }
        if (GetEntityValueType(item->type) != GetEntityValueType(type))
        {
            mvThrowPythonError(mvErrorCode::mvSourceNotCompatible, "set_value",
                "Values types do not match: " + std::to_string(dataSource), this);
            return;
        }
        _value = *static_cast<std::shared_ptr<int>*>(item->getValue());
    }

    PyObject* mvDragInt::getPyValue()
    {
        return ToPyInt(*_value);
    }

    void mvDragInt::setPyValue(PyObject* value)
    {
        *_value = ToInt(value);
    }

    void mvDragInt::draw(ImDrawList* drawlist, float x, float y)
    {

        //-----------------------------------------------------------------------------
        // pre draw
        //-----------------------------------------------------------------------------

        // show/hide
        if (!config.show)
            return;

        // focusing
        if (info.focusNextFrame)
        {
            ImGui::SetKeyboardFocusHere();
            info.focusNextFrame = false;
        }

        // cache old cursor position
        ImVec2 previousCursorPos = ImGui::GetCursorPos();

        // set cursor position if user set
        if (info.dirtyPos)
            ImGui::SetCursorPos(state.pos);

        // update widget's position state
        state.pos = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };

        // set item width
        if (config.width != 0)
            ImGui::SetNextItemWidth((float)config.width);

        // set indent
        if (config.indent > 0.0f)
            ImGui::Indent(config.indent);

        // push font if a font object is attached
        if (font)
        {
            ImFont* fontptr = static_cast<mvFont*>(font.get())->getFontPtr();
            ImGui::PushFont(fontptr);
        }

        // themes
        apply_local_theming(this);

        //-----------------------------------------------------------------------------
        // draw
        //-----------------------------------------------------------------------------
        {

            ScopedID id(uuid);

            if(!config.enabled) _disabled_value = *_value;

            if (ImGui::DragInt(info.internalLabel.c_str(), config.enabled ? _value.get() : &_disabled_value, _speed, _min, _max, _format.c_str(), _flags))
            {
                auto value = *_value;
                if(config.alias.empty())
                    mvSubmitCallback([=]() {
                    mvAddCallback(getCallback(false), uuid, ToPyInt(value), config.user_data);
                        });
                else
                    mvSubmitCallback([=]() {
                    mvAddCallback(getCallback(false), config.alias, ToPyInt(value), config.user_data);
                        });
            }
        }

        //-----------------------------------------------------------------------------
        // update state
        //-----------------------------------------------------------------------------
        UpdateAppItemState(state);

        //-----------------------------------------------------------------------------
        // post draw
        //-----------------------------------------------------------------------------

        // set cursor position to cached position
        if (info.dirtyPos)
            ImGui::SetCursorPos(previousCursorPos);

        if (config.indent > 0.0f)
            ImGui::Unindent(config.indent);

        // pop font off stack
        if (font)
            ImGui::PopFont();

        // handle popping themes
        cleanup_local_theming(this);

        if (handlerRegistry)
            handlerRegistry->checkEvents(&state);

        // handle drag & drop if used
        apply_drag_drop(this);
    }

    void mvDragFloat::handleSpecificKeywordArgs(PyObject* dict)
    {
        if (dict == nullptr)
            return;

        if (PyObject* item = PyDict_GetItemString(dict, "format")) _format = ToString(item);
        if (PyObject* item = PyDict_GetItemString(dict, "speed")) _speed = ToFloat(item);
        if (PyObject* item = PyDict_GetItemString(dict, "min_value")) _min = ToFloat(item);
        if (PyObject* item = PyDict_GetItemString(dict, "max_value")) _max = ToFloat(item);

        // helper for bit flipping
        auto flagop = [dict](const char* keyword, int flag, int& flags)
        {
            if (PyObject* item = PyDict_GetItemString(dict, keyword)) ToBool(item) ? flags |= flag : flags &= ~flag;
        };

        // flags
        flagop("clamped", ImGuiSliderFlags_ClampOnInput, _flags);
        flagop("clamped", ImGuiSliderFlags_ClampOnInput, _stor_flags);
        flagop("no_input", ImGuiSliderFlags_NoInput, _flags);
        flagop("no_input", ImGuiSliderFlags_NoInput, _stor_flags);

        if (info.enabledLastFrame)
        {
            info.enabledLastFrame = false;
            _flags = _stor_flags;
        }

        if (info.disabledLastFrame)
        {
            info.disabledLastFrame = false;
            _stor_flags = _flags;
            _flags |= ImGuiSliderFlags_NoInput;
        }

    }

    void mvDragFloat::getSpecificConfiguration(PyObject* dict)
    {
        if (dict == nullptr)
            return;

        mvPyObject py_format = ToPyString(_format);
        mvPyObject py_speed = ToPyFloat(_speed);
        mvPyObject py_min_value = ToPyFloat(_min);
        mvPyObject py_max_value = ToPyFloat(_max);

        PyDict_SetItemString(dict, "format", py_format);
        PyDict_SetItemString(dict, "speed", py_speed);
        PyDict_SetItemString(dict, "min_value", py_min_value);
        PyDict_SetItemString(dict, "max_value", py_max_value);

        // helper to check and set bit
        auto checkbitset = [dict](const char* keyword, int flag, const int& flags)
        {
            mvPyObject py_result = ToPyBool(flags & flag);
            PyDict_SetItemString(dict, keyword, py_result);
        };

        // window flags
        checkbitset("clamped", ImGuiSliderFlags_ClampOnInput, _flags);
        checkbitset("no_input", ImGuiSliderFlags_NoInput, _flags);

    }

    void mvDragInt::handleSpecificKeywordArgs(PyObject* dict)
    {
        if (dict == nullptr)
            return;

        if (PyObject* item = PyDict_GetItemString(dict, "format")) _format = ToString(item);
        if (PyObject* item = PyDict_GetItemString(dict, "speed")) _speed = ToFloat(item);
        if (PyObject* item = PyDict_GetItemString(dict, "min_value")) _min = ToInt(item);
        if (PyObject* item = PyDict_GetItemString(dict, "max_value")) _max = ToInt(item);

        // helper for bit flipping
        auto flagop = [dict](const char* keyword, int flag, int& flags)
        {
            if (PyObject* item = PyDict_GetItemString(dict, keyword)) ToBool(item) ? flags |= flag : flags &= ~flag;
        };

        // flags
        flagop("clamped", ImGuiSliderFlags_ClampOnInput, _flags);
        flagop("clamped", ImGuiSliderFlags_ClampOnInput, _stor_flags);
        flagop("no_input", ImGuiSliderFlags_NoInput, _flags);
        flagop("no_input", ImGuiSliderFlags_NoInput, _stor_flags);

        if (info.enabledLastFrame)
        {
            info.enabledLastFrame = false;
            _flags = _stor_flags;
        }

        if (info.disabledLastFrame)
        {
            info.disabledLastFrame = false;
            _stor_flags = _flags;
            _flags |= ImGuiSliderFlags_NoInput;
        }
    }

    void mvDragInt::getSpecificConfiguration(PyObject* dict)
    {
        if (dict == nullptr)
            return;

        mvPyObject py_format = ToPyString(_format);
        mvPyObject py_speed = ToPyFloat(_speed);
        mvPyObject py_min_value = ToPyInt(_min);
        mvPyObject py_max_value = ToPyInt(_max);

        PyDict_SetItemString(dict, "format", py_format);
        PyDict_SetItemString(dict, "speed", py_speed);
        PyDict_SetItemString(dict, "min_value", py_min_value);
        PyDict_SetItemString(dict, "max_value", py_max_value);

        // helper to check and set bit
        auto checkbitset = [dict](const char* keyword, int flag, const int& flags)
        {
            mvPyObject py_result = ToPyBool(flags & flag);
            PyDict_SetItemString(dict, keyword, py_result);
        };

        // window flags
        checkbitset("clamped", ImGuiSliderFlags_ClampOnInput, _flags);
        checkbitset("no_input", ImGuiSliderFlags_NoInput, _flags);

    }

}