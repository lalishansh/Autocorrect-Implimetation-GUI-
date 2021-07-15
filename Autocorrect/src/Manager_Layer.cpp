#include "Manager_Layer.h"

Manager_Layer *Manager_Layer::s_Singleton = nullptr;

Manager_Layer::Manager_Layer (const int argc, char *argv[], char *envv[])
{

}
void Manager_Layer::OnDelete ()
{

}
void Manager_Layer::OnEvent (GLCore::Event &event)
{

}
void Manager_Layer::OnUpdate (GLCore::Timestep ts)
{

}
void Manager_Layer::OnImGuiRender ()
{

}
void Manager_Layer::OnAttach ()
{

}
void Manager_Layer::OnDetach ()
{

}
void Manager_Layer::ImGuiRenderDockables ()
{

}
void Manager_Layer::ImGuiMainMenuBarItems ()
{

}
void Manager_Layer::SettingsImGuiRender ()
{

}
bool Manager_Layer::TrySaveSettings ()
{

}
void Manager_Layer::DiscardSettings ()
{

}
bool Manager_Layer::ChangeUnifiedDictionaryState (bool from, bool &to)
{

}
bool Manager_Layer::IsFocused (Text_Entity *ptr)
{

}
void Manager_Layer::ChangeFocusTo (Text_Entity *ptr)
{

}
void Manager_Layer::CloseTextEntity (Text_Entity *ptr)
{

}
void Manager_Layer::SetSignalHint (char *hint)
{

}
bool Manager_Layer::LoadTextFileToEntity (char *filePath)
{

}
bool Manager_Layer::ExtendDictionaryADictionary (SymSpell_Dictionary &handle, char *from_FilePath)
{

}