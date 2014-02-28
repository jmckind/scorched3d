////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2013
//
//    This file is part of Scorched3D.
//
//    Scorched3D is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Scorched3D is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
////////////////////////////////////////////////////////////////////////////////

#include <dialogs/GUIConsole.h>
#include <scorched3dc/ScorchedUI.h>
#include <scorched3dc/InputManager.h>
#include <client/ScorchedClient.h>
#include <console/Console.h>
#include <engine/ThreadCallback.h>
#include <common/Logger.h>

GUIConsoleClientUISyncAction::GUIConsoleClientUISyncAction(const CEGUI::String &inMsg) :
	inMsg_(inMsg)
{

}

GUIConsoleClientUISyncAction::~GUIConsoleClientUISyncAction()
{

}

void GUIConsoleClientUISyncAction::performUIAction()
{
	ScorchedClient::instance()->getConsole().addLine(true, inMsg_);
	delete this;
}

GUIConsoleTabCompleteUISyncAction::GUIConsoleTabCompleteUISyncAction(const CEGUI::String &currentText) :
	currentText_(currentText)
{

}

GUIConsoleTabCompleteUISyncAction::~GUIConsoleTabCompleteUISyncAction()
{

}

void GUIConsoleTabCompleteUISyncAction::performUIAction()
{
	std::vector<ConsoleRule *> matches;
	CEGUI::String result = ScorchedClient::instance()->getConsole().matchRule(currentText_, matches);
	if (result.length() > 0)
	{
		GUIConsole::instance()->setText(result);
	}

	GUIConsole::instance()->outputText("-------------------", false);
	std::vector<ConsoleRule *>::iterator itor;
	for (itor = matches.begin();
		itor != matches.end();
		++itor)
	{
		std::string text = (*itor)->toString();
		GUIConsole::instance()->outputText(text.c_str(), false);
	}
	
	delete this;
}

GUIConsole *GUIConsole::instance()
{
	static GUIConsole instance_;
	return &instance_;
}

GUIConsole::GUIConsole() : consoleWindow_(0), commandHistoryIndex_(0)
{
   create();
   setVisible(false);
   Logger::addLogger(this, true);
}

GUIConsole::~GUIConsole()
{
	Logger::remLogger(this);
	consoleWindow_ = 0;
}

void GUIConsole::create()
{
	CEGUI::WindowManager *pWindowManager = CEGUI::WindowManager::getSingletonPtr();
	consoleWindow_ = pWindowManager->loadLayoutFromFile("GUIConsole.layout");
 
	ScorchedUI::instance()->getInputManager().addKeyboardHandler(this);
	if (consoleWindow_)
	{
		CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(consoleWindow_);
		consoleWindow_->getChild("AddButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&GUIConsole::handle_TextSubmitted, this));

		editBox_ = static_cast<CEGUI::Editbox*>(consoleWindow_->getChild("EditBox"));
 		editBox_->subscribeEvent(
			CEGUI::Editbox::EventTextAccepted,
			CEGUI::Event::Subscriber(&GUIConsole::handle_TextSubmitted,this));
		editBox_->subscribeEvent(
			CEGUI::Editbox::EventKeyDown,
			CEGUI::Event::Subscriber(&GUIConsole::handle_keyDownSubmitted,this));
		outputWindow_ = static_cast<CEGUI::Listbox*>(consoleWindow_->getChild("DisplayLines"));
	}
}

bool GUIConsole::handle_TextSubmitted(const CEGUI::EventArgs &e)
{
	const CEGUI::String msg = editBox_->getText();
	editBox_->setText("");

	GUIConsoleClientUISyncAction *clientCallback = new GUIConsoleClientUISyncAction(msg);
	ScorchedClient::getClientUISyncExternal().addActionFromUI(clientCallback);

	return true;
}

bool GUIConsole::handle_keyDownSubmitted(const CEGUI::EventArgs &e)
{
	if (editBox_->hasInputFocus())
	{
		CEGUI::KeyEventArgs &kargs = (CEGUI::KeyEventArgs &) e;
		switch (kargs.scancode)
		{
		case CEGUI::Key::Tab:
			tabComplete();
			return true;
		case CEGUI::Key::Escape:
			editBox_->setText("");
			return true;
		case CEGUI::Key::PageUp:
			outputWindow_->getVertScrollbar()->scrollBackwardsByPage();
			return true;
		case CEGUI::Key::PageDown:
			outputWindow_->getVertScrollbar()->scrollForwardsByPage();
			return true;
		case CEGUI::Key::ArrowUp:
			{
				if (commandHistoryIndex_ < (int) commandHistory_.size())
				{
					commandHistoryIndex_++;
					setText(commandHistory_[commandHistory_.size() - commandHistoryIndex_]);
				}
			}
			return true;
		case CEGUI::Key::ArrowDown:
			{
				if (commandHistoryIndex_ > 0)
				{
					commandHistoryIndex_--;
					if (commandHistoryIndex_ == 0)
					{
						setText("");
					}
					else 
					{
						setText(commandHistory_[commandHistory_.size() - commandHistoryIndex_]);
					}
				}
			}
			return true;
		case CEGUI::Key::Home:
			editBox_->setCaretIndex(0);
			return true;
		case CEGUI::Key::End:
			editBox_->setCaretIndex(99999);
			return true;
		case CEGUI::Key::Grave:
			setVisible(false);
			return true;
		}
	}
	return false;
}

void GUIConsole::tabComplete()
{
	GUIConsoleTabCompleteUISyncAction *tabComplete = 
		new GUIConsoleTabCompleteUISyncAction(editBox_->getText());
	ScorchedClient::getClientUISyncExternal().addActionFromUI(tabComplete);
}

void GUIConsole::setText(const CEGUI::String &text)
{
	ENSURE_UI_THREAD
	editBox_->setText(text);
	editBox_->setCaretIndex(text.length());
}

void GUIConsole::outputText(const CEGUI::String &inMsg, bool command)
{ 
	ENSURE_UI_THREAD
	const CEGUI::Colour red = CEGUI::Colour(0xFF0000FF);
	const CEGUI::Colour white  = CEGUI::Colour(0xFF3333FF);

	CEGUI::ListboxTextItem *newItem = 0;
	if (command)
	{
		newItem = new CEGUI::ListboxTextItem("> " + inMsg);
		newItem->setTextColours(red); 
		commandHistoryIndex_ = 0;
		commandHistory_.push_back(inMsg);
	}
	else 
	{
		newItem = new CEGUI::ListboxTextItem("  " + inMsg);
		newItem->setTextColours(white); 
	}
	
	//, CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
	outputWindow_->addItem(newItem);
	outputWindow_->ensureItemIsVisible(outputWindow_->getItemCount());
}

void GUIConsole::logMessage(LoggerInfo &info)
{
	std::string log = S3D::formatStringBuffer("%s - %s", info.getTime(), info.getMessage());
	outputText(log, false);
}

void GUIConsole::setVisible(bool visible)
{
    consoleWindow_->setVisible(visible);
 
    if(visible) 
	{
		editBox_->activate();
		editBox_->setText("");
	}
    else editBox_->deactivate();
}
 
bool GUIConsole::isVisible()
{
    return consoleWindow_->isVisible();
}

void GUIConsole::keyPressed(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_GRAVE)
	{
		setVisible(!isVisible());
	}
}

void GUIConsole::keyReleased(const OIS::KeyEvent &arg)
{

}