#include "main.h"

#include "keyboardhistory.h"
#include "util/CJavaWrapper.h"
#include "gui/gui.h"
#include "game/game.h"
#include "keyboard.h"
#include "scrollbar.h"

extern CGUI* pGUI;

CKeyBoard::CKeyBoard()
{
	Log("Initalizing KeyBoard..");

	ImGuiIO& io = ImGui::GetIO();
	m_Size = ImVec2(io.DisplaySize.x, io.DisplaySize.y * 0.55);
	m_Pos = ImVec2(0, io.DisplaySize.y * (1 - 0.55));
	m_fFontSize = pGUI->ScaleY(70.0f);
	m_fKeySizeY = m_Size.y / 5;


	Log("Size: %f, %f. Pos: %f, %f", m_Size.x, m_Size.y, m_Pos.x, m_Pos.y);
	Log("font size: %f. Key's height: %f", m_fFontSize, m_fKeySizeY);

	m_bEnable = false;
	m_iLayout = LAYOUT_ENG;
	m_iCase = LOWER_CASE;
	m_iPushedKey = -1;

	m_utf8Input[0] = '\0';
	m_iInputOffset = 0;

	m_pkHistory = new CKeyBoardHistory();

	InitENG();
	InitRU();
	InitNUM();

	m_bNewKeyboard = true;

	Log("KeyBoard inited");
}

CKeyBoard::~CKeyBoard()
{
}

void CKeyBoard::Render()
{
	if (!m_bEnable || m_bNewKeyboard) return;

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 vecButSize = ImVec2(ImGui::GetFontSize() * 4, ImGui::GetFontSize() * 2.5);
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - vecButSize.x, io.DisplaySize.y / 2 - vecButSize.y * 3));
	ImGui::Begin("###keyboard", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::Button("UP", vecButSize))
	{
		m_pkHistory->PageUp();
	}

	if (ImGui::Button("DOWN", vecButSize))
	{
		m_pkHistory->PageDown();
	}

	ImGui::End();

	// background
	ImGui::GetOverlayDrawList()->AddRectFilled(m_Pos, ImVec2(m_Size.x, io.DisplaySize.y), 0xB0000000);

	// input string
	if (IsHidden())
	{
		char _utf8DialogInputBuffer[100 * 3 + 1];
		strcpy(_utf8DialogInputBuffer, m_utf8Input);

		for (int i = 0; i < strlen(_utf8DialogInputBuffer); i++)
		{
			if (_utf8DialogInputBuffer[i] == '\0')
				break;
			_utf8DialogInputBuffer[i] = '*';
		}
		ImGui::GetOverlayDrawList()->AddText(pGUI->GetFont(), m_fFontSize,
			ImVec2(m_Pos.x + m_Size.x * 0.02, m_Pos.y + m_Pos.y * 0.05), 0xFFFFFFFF, _utf8DialogInputBuffer);
	}
	else
		ImGui::GetOverlayDrawList()->AddText(pGUI->GetFont(), m_fFontSize,
			ImVec2(m_Pos.x + m_Size.x * 0.02, m_Pos.y + m_Pos.y * 0.05), 0xFFFFFFFF, m_utf8Input);

	// dividing line
	ImGui::GetOverlayDrawList()->AddLine(
		ImVec2(m_Pos.x, m_Pos.y + m_fKeySizeY),
		ImVec2(m_Size.x, m_Pos.y + m_fKeySizeY), 0xFF3291F5);

	float fKeySizeY = m_fKeySizeY;

	for (int i = 0; i < 4; i++)
	{
		for (auto key : m_Rows[m_iLayout][i])
		{
			if (key.id == m_iPushedKey && key.type != KEY_SPACE && !IsHidden())
				ImGui::GetOverlayDrawList()->AddRectFilled(
					key.pos,
					ImVec2(key.pos.x + key.width, key.pos.y + fKeySizeY),
					0xFF3291F5);

			switch (key.type)
			{
			case KEY_DEFAULT:
				ImGui::GetOverlayDrawList()->AddText(pGUI->GetFont(), m_fFontSize, key.symPos, 0xFFFFFFFF, key.name[m_iCase]);
				break;

			case KEY_SHIFT:
				ImGui::GetOverlayDrawList()->AddTriangleFilled(
					ImVec2(key.pos.x + key.width * 0.37, key.pos.y + fKeySizeY * 0.50),
					ImVec2(key.pos.x + key.width * 0.50, key.pos.y + fKeySizeY * 0.25),
					ImVec2(key.pos.x + key.width * 0.63, key.pos.y + fKeySizeY * 0.50),
					m_iCase == LOWER_CASE ? 0xFF8A8886 : 0xFF3291F5);
				ImGui::GetOverlayDrawList()->AddRectFilled(
					ImVec2(key.pos.x + key.width * 0.44, key.pos.y + fKeySizeY * 0.5 - 1),
					ImVec2(key.pos.x + key.width * 0.56, key.pos.y + fKeySizeY * 0.68),
					m_iCase == LOWER_CASE ? 0xFF8A8886 : 0xFF3291F5);
				break;

			case KEY_BACKSPACE:
				static ImVec2 points[5];
				points[0] = ImVec2(key.pos.x + key.width * 0.35, key.pos.y + fKeySizeY * 0.5);
				points[1] = ImVec2(key.pos.x + key.width * 0.45, key.pos.y + fKeySizeY * 0.25);
				points[2] = ImVec2(key.pos.x + key.width * 0.65, key.pos.y + fKeySizeY * 0.25);
				points[3] = ImVec2(key.pos.x + key.width * 0.65, key.pos.y + fKeySizeY * 0.65);
				points[4] = ImVec2(key.pos.x + key.width * 0.45, key.pos.y + fKeySizeY * 0.65);
				ImGui::GetOverlayDrawList()->AddConvexPolyFilled(points, 5, 0xFF8A8886);
				break;

			case KEY_SWITCH:
				ImGui::GetOverlayDrawList()->AddText(pGUI->GetFont(), m_fFontSize, key.symPos, 0xFFFFFFFF, "lang");
				break;

			case KEY_SPACE:
				ImGui::GetOverlayDrawList()->AddRectFilled(
					ImVec2(key.pos.x + key.width * 0.07, key.pos.y + fKeySizeY * 0.3),
					ImVec2(key.pos.x + key.width * 0.93, key.pos.y + fKeySizeY * 0.7),
					key.id == m_iPushedKey && !IsHidden() ? 0xFF3291F5 : 0xFF8A8886);
				break;

			case KEY_SEND:
				ImGui::GetOverlayDrawList()->AddTriangleFilled(
					ImVec2(key.pos.x + key.width * 0.3, key.pos.y + fKeySizeY * 0.25),
					ImVec2(key.pos.x + key.width * 0.3, key.pos.y + fKeySizeY * 0.75),
					ImVec2(key.pos.x + key.width * 0.7, key.pos.y + fKeySizeY * 0.50),
					0xFF8A8886);
				break;
			}
		}
	}
}

void CKeyBoard::Open(keyboard_callback* handler, bool bHiden)
{
	if (handler == nullptr) return;

	Close();
	if (m_pkHistory)
	{
		m_pkHistory->m_iCounter = 0;
	}
	m_pHandler = handler;
	m_bEnable = true;
	m_bInputFlag = bHiden;
	
	g_pJavaWrapper->HideHudFeatures();

	if (m_bNewKeyboard)
	{
		if (g_pJavaWrapper)
		{
			g_pJavaWrapper->ShowInputLayout();
		}
	}
}

void CKeyBoard::Close()
{
	m_bEnable = false;

	m_sInput.clear();
	m_iInputOffset = 0;
	m_utf8Input[0] = 0;
	m_iCase = LOWER_CASE;
	m_iPushedKey = -1;
	m_pHandler = nullptr;
	
	g_pJavaWrapper->ShowHudFeatures();

	if (m_bNewKeyboard)
	{
		if (g_pJavaWrapper)
		{
			g_pJavaWrapper->HideInputLayout();
		}
	}

	return;
}
#include "util/CJavaWrapper.h"
bool CKeyBoard::OnTouchEvent(int type, bool multi, int x, int y)
{
	static bool bWannaClose = false;

	if (!m_bEnable) return true;

	ImGuiIO& io = ImGui::GetIO();

	if (x >= io.DisplaySize.x - ImGui::GetFontSize() * 4 && y >= io.DisplaySize.y / 2 - (ImGui::GetFontSize() * 2.5) * 3 && y <= io.DisplaySize.y / 2) // keys
	{
		return true;
	}


	if (pScrollbar)
	{
		if (!pScrollbar->OnTouchEvent(type, multi, x, y))
		{
			return false;
		}
	}

	static bool bWannaCopy = false;
	static uint32_t uiTouchTick = 0;

	ImVec2 leftCorner(m_Pos.x, m_Pos.y);
	ImVec2 rightCorner(m_Size.x, m_Pos.y + m_fKeySizeY);

	if (g_pJavaWrapper)
	{
		if (type == TOUCH_PUSH && x >= leftCorner.x && y >= leftCorner.y && x <= rightCorner.x && y <= rightCorner.y)
		{
			if (bWannaCopy && GetTickCount() - uiTouchTick <= 150)
			{
				std::string msg = g_pJavaWrapper->GetClipboardString();
				for (int i = 0; i < msg.size(); i++)
				{
					AddCharToInput((char)msg[i]);
				}
				bWannaCopy = false;
			}
			else
			{
				bWannaCopy = true;
				uiTouchTick = GetTickCount();
			}
		}

		if (type == TOUCH_POP)
		{
			if (GetTickCount() - uiTouchTick <= 150 && bWannaCopy)
			{
				bWannaCopy = true;
				uiTouchTick = GetTickCount();
			}
			else
			{
				bWannaCopy = false;
			}
		}
	}

	if (type == TOUCH_PUSH && y < m_Pos.y)
	{
		bWannaClose = true;
	}
	if (type == TOUCH_POP && y < m_Pos.y && bWannaClose)
	{
		bWannaClose = false;
		Close();
		return false;
	}

	m_iPushedKey = -1;

	kbKey* key = GetKeyFromPos(x, y);
	//Log("CKeyBoard::OnTouchEvent(%d, %d, %d, %d)", type, multi, x, y);
	if (!key) return false;

	switch (type)
	{
	case TOUCH_PUSH:
		m_iPushedKey = key->id;
		break;

	case TOUCH_MOVE:
		m_iPushedKey = key->id;
		break;

	case TOUCH_POP:
		HandleInput(*key);
		break;
	}
	delete key;
	return false;
}

void CKeyBoard::HandleInput(kbKey& key)
{
	switch (key.type)
	{
	case KEY_DEFAULT:
	case KEY_SPACE:
		AddCharToInput(key.code[m_iCase]);
		break;

	case KEY_SWITCH:
		m_iLayout++;
		if (m_iLayout >= 3) m_iLayout = 0;
		m_iCase = LOWER_CASE;
		break;

	case KEY_BACKSPACE:
		DeleteCharFromInput();
		break;

	case KEY_SHIFT:
		m_iCase ^= 1;
		break;

	case KEY_SEND:
		Send();
		break;
	}
}

void CKeyBoard::AddCharToInput(uint16_t sym)
{
	if (m_sInput.length() < MAX_INPUT_LEN && sym)
	{
		// Nếu bạn vẫn đang dùng cp1251 byte-based string thì cần ép về byte
		// để giữ đúng logic cũ của project.
		m_sInput.push_back((char)(sym & 0xFF));

		cp1251_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset]);

	check:
		ImVec2 textSize = pGUI->GetFont()->CalcTextSizeA(
			m_fFontSize, FLT_MAX, 0.0f, m_utf8Input, nullptr, nullptr);

		if (textSize.x >= (m_Size.x - (m_Size.x * 0.04f)))
		{
			m_iInputOffset++;
			cp1251_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset]);
			goto check;
		}
	}
}

void CKeyBoard::DeleteCharFromInput()
{
	if (!m_sInput.length()) return;

	ImVec2 textSize;
	m_sInput.pop_back();

check:
	if (m_iInputOffset == 0) goto ret;
	cp1251_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset - 1]);
	textSize = pGUI->GetFont()->CalcTextSizeA(m_fFontSize, FLT_MAX, 0.0f, m_utf8Input, nullptr, nullptr);

	if (textSize.x <= (m_Size.x - (m_Size.x * 0.04)))
	{
		m_iInputOffset--;
		goto check;
	}
	else
	{
	ret:
		cp1251_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset]);
		return;
	}
}

void CKeyBoard::Send()
{
	if (m_pHandler)
	{
		m_pHandler(m_sInput.c_str());
		if (m_pkHistory && !IsHidden()) m_pkHistory->AddStringToHistory(m_sInput);
	}
	Close();
}

kbKey* CKeyBoard::GetKeyFromPos(int x, int y)
{
	int iRow = (y - m_Pos.y) / m_fKeySizeY;

	if (iRow <= 0) return nullptr;

	for (auto key : m_Rows[m_iLayout][iRow - 1])
	{
		if (x >= key.pos.x && x <= (key.pos.x + key.width))
		{
			kbKey* pKey = new kbKey;
			memcpy((void*)pKey, (const void*)& key, sizeof(kbKey));
			return pKey;
		}
	}

	Log("UNKNOWN KEY");
	return nullptr;
}

void CKeyBoard::InitENG()
{
	Log(__FUNCTION__);
	ImVec2 curPos;
	std::vector<kbKey>* row = nullptr;
	std::vector<kbKey>::iterator it;

	float defWidth = m_Size.x / 10;
	struct kbKey key;
	key.type = KEY_HISTORY;
	key.id = 0;

	// 1-�� ���
	row = &m_Rows[LAYOUT_ENG][0];
	curPos = ImVec2(0, m_Pos.y + m_fKeySizeY);

	key.type = KEY_DEFAULT;
	// q/Q
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'q';
	key.code[UPPER_CASE] = 'Q';
	cp1251_to_utf8(key.name[LOWER_CASE], "q");
	cp1251_to_utf8(key.name[UPPER_CASE], "Q");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// w/W
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'w';
	key.code[UPPER_CASE] = 'W';
	cp1251_to_utf8(key.name[LOWER_CASE], "w");
	cp1251_to_utf8(key.name[UPPER_CASE], "W");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// e/E
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'e';
	key.code[UPPER_CASE] = 'E';
	cp1251_to_utf8(key.name[LOWER_CASE], "e");
	cp1251_to_utf8(key.name[UPPER_CASE], "E");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// r/R
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'r';
	key.code[UPPER_CASE] = 'R';
	cp1251_to_utf8(key.name[LOWER_CASE], "r");
	cp1251_to_utf8(key.name[UPPER_CASE], "R");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// t/T
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 't';
	key.code[UPPER_CASE] = 'T';
	cp1251_to_utf8(key.name[LOWER_CASE], "t");
	cp1251_to_utf8(key.name[UPPER_CASE], "T");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// y/Y
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'y';
	key.code[UPPER_CASE] = 'Y';
	cp1251_to_utf8(key.name[LOWER_CASE], "y");
	cp1251_to_utf8(key.name[UPPER_CASE], "Y");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// u/U
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'u';
	key.code[UPPER_CASE] = 'U';
	cp1251_to_utf8(key.name[LOWER_CASE], "u");
	cp1251_to_utf8(key.name[UPPER_CASE], "U");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// i/I
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'i';
	key.code[UPPER_CASE] = 'I';
	cp1251_to_utf8(key.name[LOWER_CASE], "i");
	cp1251_to_utf8(key.name[UPPER_CASE], "I");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// o/O
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'o';
	key.code[UPPER_CASE] = 'O';
	cp1251_to_utf8(key.name[LOWER_CASE], "o");
	cp1251_to_utf8(key.name[UPPER_CASE], "O");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// p/P
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'p';
	key.code[UPPER_CASE] = 'P';
	cp1251_to_utf8(key.name[LOWER_CASE], "p");
	cp1251_to_utf8(key.name[UPPER_CASE], "P");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 2-� ���
	row = &m_Rows[LAYOUT_ENG][1];
	curPos.x = defWidth * 0.5;
	curPos.y += m_fKeySizeY;

	// a/A
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'a';
	key.code[UPPER_CASE] = 'A';
	cp1251_to_utf8(key.name[LOWER_CASE], "a");
	cp1251_to_utf8(key.name[UPPER_CASE], "A");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// s/S
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 's';
	key.code[UPPER_CASE] = 'S';
	cp1251_to_utf8(key.name[LOWER_CASE], "s");
	cp1251_to_utf8(key.name[UPPER_CASE], "S");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// d/D
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'd';
	key.code[UPPER_CASE] = 'D';
	cp1251_to_utf8(key.name[LOWER_CASE], "d");
	cp1251_to_utf8(key.name[UPPER_CASE], "D");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// f/F
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'f';
	key.code[UPPER_CASE] = 'F';
	cp1251_to_utf8(key.name[LOWER_CASE], "f");
	cp1251_to_utf8(key.name[UPPER_CASE], "F");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// g/G
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'g';
	key.code[UPPER_CASE] = 'G';
	cp1251_to_utf8(key.name[LOWER_CASE], "g");
	cp1251_to_utf8(key.name[UPPER_CASE], "G");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// h/H
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'h';
	key.code[UPPER_CASE] = 'H';
	cp1251_to_utf8(key.name[LOWER_CASE], "h");
	cp1251_to_utf8(key.name[UPPER_CASE], "H");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// j/J
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'j';
	key.code[UPPER_CASE] = 'J';
	cp1251_to_utf8(key.name[LOWER_CASE], "j");
	cp1251_to_utf8(key.name[UPPER_CASE], "J");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// k/K
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'k';
	key.code[UPPER_CASE] = 'K';
	cp1251_to_utf8(key.name[LOWER_CASE], "k");
	cp1251_to_utf8(key.name[UPPER_CASE], "K");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// l/L
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'l';
	key.code[UPPER_CASE] = 'L';
	cp1251_to_utf8(key.name[LOWER_CASE], "l");
	cp1251_to_utf8(key.name[UPPER_CASE], "L");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 3-� ���
	row = &m_Rows[LAYOUT_ENG][2];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	// Shift
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth * 1.5;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SHIFT;
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	key.type = KEY_DEFAULT;

	// z/Z
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'z';
	key.code[UPPER_CASE] = 'Z';
	cp1251_to_utf8(key.name[LOWER_CASE], "z");
	cp1251_to_utf8(key.name[UPPER_CASE], "Z");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// x/X
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'x';
	key.code[UPPER_CASE] = 'X';
	cp1251_to_utf8(key.name[LOWER_CASE], "x");
	cp1251_to_utf8(key.name[UPPER_CASE], "X");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// c/C
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'c';
	key.code[UPPER_CASE] = 'C';
	cp1251_to_utf8(key.name[LOWER_CASE], "c");
	cp1251_to_utf8(key.name[UPPER_CASE], "C");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// v/V
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'v';
	key.code[UPPER_CASE] = 'V';
	cp1251_to_utf8(key.name[LOWER_CASE], "v");
	cp1251_to_utf8(key.name[UPPER_CASE], "V");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// b/B
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'b';
	key.code[UPPER_CASE] = 'B';
	cp1251_to_utf8(key.name[LOWER_CASE], "b");
	cp1251_to_utf8(key.name[UPPER_CASE], "B");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// n/N
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'n';
	key.code[UPPER_CASE] = 'N';
	cp1251_to_utf8(key.name[LOWER_CASE], "n");
	cp1251_to_utf8(key.name[UPPER_CASE], "N");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// m/M
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 'm';
	key.code[UPPER_CASE] = 'M';
	cp1251_to_utf8(key.name[LOWER_CASE], "m");
	cp1251_to_utf8(key.name[UPPER_CASE], "M");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// delete
	key.pos = curPos;
	key.symPos = ImVec2(0, 0);
	key.width = defWidth * 1.5;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_BACKSPACE;
	key.id++;
	it = row->begin();
	row->insert(it, key);

	// 4-� ������
	row = &m_Rows[LAYOUT_ENG][3];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	key.type = KEY_DEFAULT;

	// slash (/)
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '/';
	key.code[UPPER_CASE] = '/';
	cp1251_to_utf8(key.name[LOWER_CASE], "/");
	cp1251_to_utf8(key.name[UPPER_CASE], "/");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// comma (,)
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ',';
	key.code[UPPER_CASE] = ',';
	cp1251_to_utf8(key.name[LOWER_CASE], ",");
	cp1251_to_utf8(key.name[UPPER_CASE], ",");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// switch language
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.2, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SWITCH;
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// Space
	key.pos = curPos;
	key.symPos = ImVec2(0, 0);
	key.width = defWidth * 4;
	key.code[LOWER_CASE] = ' ';
	key.code[UPPER_CASE] = ' ';
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SPACE;
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	key.type = KEY_DEFAULT;

	// ?
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '?';
	key.code[UPPER_CASE] = '?';
	cp1251_to_utf8(key.name[LOWER_CASE], "?");
	cp1251_to_utf8(key.name[UPPER_CASE], "?");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// !
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '!';
	key.code[UPPER_CASE] = '!';
	cp1251_to_utf8(key.name[LOWER_CASE], "!");
	cp1251_to_utf8(key.name[UPPER_CASE], "!");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// Send
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SEND;
	key.id++;
	it = row->begin();
	row->insert(it, key);
	//curPos.x += key.width;

	return;
}

void CKeyBoard::InitRU()
{
	Log(__FUNCTION__);

	struct KeyDef
	{
		uint8_t lower;
		uint8_t upper;
		const char* lowerText;
		const char* upperText;
	};

	auto addKey = [&](std::vector<kbKey>* row, float& x, float y, float width,
		uint8_t lower, uint8_t upper,
		const char* lowerText, const char* upperText,
		int type = KEY_DEFAULT)
	{
		kbKey key{};
		key.pos = ImVec2(x, y);
		key.symPos = ImVec2(x + width * 0.4f, y + m_fKeySizeY * 0.2f);
		key.width = width;
		key.code[LOWER_CASE] = (char)lower;
		key.code[UPPER_CASE] = (char)upper;
		key.type = type;
		key.id = (int)row->size();

		if (lowerText && lowerText[0] != '\0')
			cp1251_to_utf8(key.name[LOWER_CASE], lowerText);
		else
			key.name[LOWER_CASE][0] = '\0';

		if (upperText && upperText[0] != '\0')
			cp1251_to_utf8(key.name[UPPER_CASE], upperText);
		else
			key.name[UPPER_CASE][0] = '\0';

		row->push_back(key);
		x += width;
	};

	const float defWidthRow1 = m_Size.x / 11.0f;
	const float defWidthRow2 = m_Size.x / 11.0f;
	const float defWidthRow3 = m_Size.x / 11.0f;
	const float defWidthRow4 = m_Size.x / 10.0f;

	// Row 0
	{
		std::vector<kbKey>* row = &m_Rows[LAYOUT_RUS][0];
		row->clear();

		float x = 0.0f;
		float y = m_Pos.y + m_fKeySizeY;

		const KeyDef keys[] = {
			{0xE9, 0xC9, "\xE9", "\xC9"}, // й/Й
			{0xF6, 0xD6, "\xF6", "\xD6"}, // ц/Ц
			{0xF3, 0xD3, "\xF3", "\xD3"}, // у/У
			{0xEA, 0xCA, "\xEA", "\xCA"}, // к/К
			{0xE5, 0xC5, "\xE5", "\xC5"}, // е/Е
			{0xED, 0xCD, "\xED", "\xCD"}, // н/Н
			{0xE3, 0xC3, "\xE3", "\xC3"}, // г/Г
			{0xF8, 0xD8, "\xF8", "\xD8"}, // ш/Ш
			{0xF9, 0xD9, "\xF9", "\xD9"}, // щ/Щ
			{0xE7, 0xC7, "\xE7", "\xC7"}, // з/З
			{0xF5, 0xD5, "\xF5", "\xD5"}, // х/Х
		};

		for (const auto& k : keys)
			addKey(row, x, y, defWidthRow1, k.lower, k.upper, k.lowerText, k.upperText);

		addKey(row, x, y, defWidthRow1, 0xFA, 0xDA, "\xFA", "\xDA"); // ъ/Ъ
	}

	// Row 1
	{
		std::vector<kbKey>* row = &m_Rows[LAYOUT_RUS][1];
		row->clear();

		float x = 0.0f;
		float y = m_Pos.y + m_fKeySizeY * 2.0f;

		const KeyDef keys[] = {
			{0xF4, 0xD4, "\xF4", "\xD4"}, // ф/Ф
			{0xFB, 0xDB, "\xFB", "\xDB"}, // ы/Ы
			{0xE2, 0xC2, "\xE2", "\xC2"}, // в/В
			{0xE0, 0xC0, "\xE0", "\xC0"}, // а/А
			{0xEF, 0xCF, "\xEF", "\xCF"}, // п/П
			{0xF0, 0xD0, "\xF0", "\xD0"}, // р/Р
			{0xEE, 0xCE, "\xEE", "\xCE"}, // о/О
			{0xEB, 0xCB, "\xEB", "\xCB"}, // л/Л
			{0xE4, 0xC4, "\xE4", "\xC4"}, // д/Д
			{0xE6, 0xC6, "\xE6", "\xC6"}, // ж/Ж
			{0xFD, 0xDD, "\xFD", "\xDD"}, // э/Э
		};

		for (const auto& k : keys)
			addKey(row, x, y, defWidthRow2, k.lower, k.upper, k.lowerText, k.upperText);

		addKey(row, x, y, defWidthRow2, 0, 0, "", "", KEY_BACKSPACE);
	}

	// Row 2
	{
		std::vector<kbKey>* row = &m_Rows[LAYOUT_RUS][2];
		row->clear();

		float x = 0.0f;
		float y = m_Pos.y + m_fKeySizeY * 3.0f;

		addKey(row, x, y, defWidthRow3 * 1.5f, 0, 0, "", "", KEY_SHIFT);

		const KeyDef keys[] = {
			{0xFF, 0xDF, "\xFF", "\xDF"}, // я/Я
			{0xF7, 0xD7, "\xF7", "\xD7"}, // ч/Ч
			{0xF1, 0xD1, "\xF1", "\xD1"}, // с/С
			{0xEC, 0xCC, "\xEC", "\xCC"}, // м/М
			{0xE8, 0xC8, "\xE8", "\xC8"}, // и/И
			{0xF2, 0xD2, "\xF2", "\xD2"}, // т/Т
			{0xFC, 0xDC, "\xFC", "\xDC"}, // ь/Ь
			{0xE1, 0xC1, "\xE1", "\xC1"}, // б/Б
			{0xFE, 0xDE, "\xFE", "\xDE"}, // ю/Ю
		};

		for (const auto& k : keys)
			addKey(row, x, y, defWidthRow3, k.lower, k.upper, k.lowerText, k.upperText);

		addKey(row, x, y, defWidthRow3, 0, 0, "", "", KEY_BACKSPACE);
	}

	// Row 3
	{
		std::vector<kbKey>* row = &m_Rows[LAYOUT_RUS][3];
		row->clear();

		float x = 0.0f;
		float y = m_Pos.y + m_fKeySizeY * 4.0f;

		addKey(row, x, y, defWidthRow4, '/', '/', "/", "/");
		addKey(row, x, y, defWidthRow4, ',', ',', ",", ",");
		addKey(row, x, y, defWidthRow4, 0, 0, "", "", KEY_SWITCH);
		addKey(row, x, y, defWidthRow4 * 4.0f, ' ', ' ', "", "", KEY_SPACE);
		addKey(row, x, y, defWidthRow4, '?', '?', "?", "?");
		addKey(row, x, y, defWidthRow4, '!', '!', "!", "!");
		addKey(row, x, y, defWidthRow4, 0, 0, "", "", KEY_SEND);
	}
}

void CKeyBoard::InitNUM()
{
	Log(__FUNCTION__);
	ImVec2 curPos;
	std::vector<kbKey>* row = nullptr;
	std::vector<kbKey>::iterator it;
	float defWidth = m_Size.x / 10;

	struct kbKey key;
	key.type = KEY_DEFAULT;
	key.id = 0;

	// 1-�� ���
	row = &m_Rows[LAYOUT_NUM][0];
	curPos = ImVec2(0, m_Pos.y + m_fKeySizeY);

	// 1
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '1';
	key.code[UPPER_CASE] = '1';
	cp1251_to_utf8(key.name[LOWER_CASE], "1");
	cp1251_to_utf8(key.name[UPPER_CASE], "1");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 2
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '2';
	key.code[UPPER_CASE] = '2';
	cp1251_to_utf8(key.name[LOWER_CASE], "2");
	cp1251_to_utf8(key.name[UPPER_CASE], "2");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 3
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '3';
	key.code[UPPER_CASE] = '3';
	cp1251_to_utf8(key.name[LOWER_CASE], "3");
	cp1251_to_utf8(key.name[UPPER_CASE], "3");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 4
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '4';
	key.code[UPPER_CASE] = '4';
	cp1251_to_utf8(key.name[LOWER_CASE], "4");
	cp1251_to_utf8(key.name[UPPER_CASE], "4");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 5
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '5';
	key.code[UPPER_CASE] = '5';
	cp1251_to_utf8(key.name[LOWER_CASE], "5");
	cp1251_to_utf8(key.name[UPPER_CASE], "5");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 6
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '6';
	key.code[UPPER_CASE] = '6';
	cp1251_to_utf8(key.name[LOWER_CASE], "6");
	cp1251_to_utf8(key.name[UPPER_CASE], "6");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 7
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '7';
	key.code[UPPER_CASE] = '7';
	cp1251_to_utf8(key.name[LOWER_CASE], "7");
	cp1251_to_utf8(key.name[UPPER_CASE], "7");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 8
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '8';
	key.code[UPPER_CASE] = '8';
	cp1251_to_utf8(key.name[LOWER_CASE], "8");
	cp1251_to_utf8(key.name[UPPER_CASE], "8");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 9
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '9';
	key.code[UPPER_CASE] = '9';
	cp1251_to_utf8(key.name[LOWER_CASE], "9");
	cp1251_to_utf8(key.name[UPPER_CASE], "9");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 0
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '0';
	key.code[UPPER_CASE] = '0';
	cp1251_to_utf8(key.name[LOWER_CASE], "0");
	cp1251_to_utf8(key.name[UPPER_CASE], "0");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 2-� ���
	row = &m_Rows[LAYOUT_NUM][1];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	// @
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '@';
	key.code[UPPER_CASE] = '@';
	cp1251_to_utf8(key.name[LOWER_CASE], "@");
	cp1251_to_utf8(key.name[UPPER_CASE], "@");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// #
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '#';
	key.code[UPPER_CASE] = '#';
	cp1251_to_utf8(key.name[LOWER_CASE], "#");
	cp1251_to_utf8(key.name[UPPER_CASE], "#");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// $
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '$';
	key.code[UPPER_CASE] = '$';
	cp1251_to_utf8(key.name[LOWER_CASE], "$");
	cp1251_to_utf8(key.name[UPPER_CASE], "$");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// %
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '%';
	key.code[UPPER_CASE] = '%';
	cp1251_to_utf8(key.name[LOWER_CASE], "%");
	cp1251_to_utf8(key.name[UPPER_CASE], "%");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// "
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '"';
	key.code[UPPER_CASE] = '"';
	cp1251_to_utf8(key.name[LOWER_CASE], "\"");
	cp1251_to_utf8(key.name[UPPER_CASE], "\"");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// *
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '*';
	key.code[UPPER_CASE] = '*';
	cp1251_to_utf8(key.name[LOWER_CASE], "*");
	cp1251_to_utf8(key.name[UPPER_CASE], "*");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// (
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '(';
	key.code[UPPER_CASE] = '(';
	cp1251_to_utf8(key.name[LOWER_CASE], "(");
	cp1251_to_utf8(key.name[UPPER_CASE], "(");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// )
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ')';
	key.code[UPPER_CASE] = ')';
	cp1251_to_utf8(key.name[LOWER_CASE], ")");
	cp1251_to_utf8(key.name[UPPER_CASE], ")");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// -
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '-';
	key.code[UPPER_CASE] = '-';
	cp1251_to_utf8(key.name[LOWER_CASE], "-");
	cp1251_to_utf8(key.name[UPPER_CASE], "-");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// _
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '_';
	key.code[UPPER_CASE] = '_';
	cp1251_to_utf8(key.name[LOWER_CASE], "_");
	cp1251_to_utf8(key.name[UPPER_CASE], "_");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// 3-� ���
	row = &m_Rows[LAYOUT_NUM][2];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	// .
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '.';
	key.code[UPPER_CASE] = '.';
	cp1251_to_utf8(key.name[LOWER_CASE], ".");
	cp1251_to_utf8(key.name[UPPER_CASE], ".");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// :
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ':';
	key.code[UPPER_CASE] = ':';
	cp1251_to_utf8(key.name[LOWER_CASE], ":");
	cp1251_to_utf8(key.name[UPPER_CASE], ":");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// ;
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ';';
	key.code[UPPER_CASE] = ';';
	cp1251_to_utf8(key.name[LOWER_CASE], ";");
	cp1251_to_utf8(key.name[UPPER_CASE], ";");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// +
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '+';
	key.code[UPPER_CASE] = '+';
	cp1251_to_utf8(key.name[LOWER_CASE], "+");
	cp1251_to_utf8(key.name[UPPER_CASE], "+");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// =
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '=';
	key.code[UPPER_CASE] = '=';
	cp1251_to_utf8(key.name[LOWER_CASE], "=");
	cp1251_to_utf8(key.name[UPPER_CASE], "=");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// <
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '<';
	key.code[UPPER_CASE] = '<';
	cp1251_to_utf8(key.name[LOWER_CASE], "<");
	cp1251_to_utf8(key.name[UPPER_CASE], "<");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// >
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '>';
	key.code[UPPER_CASE] = '>';
	cp1251_to_utf8(key.name[LOWER_CASE], ">");
	cp1251_to_utf8(key.name[UPPER_CASE], ">");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// [
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '[';
	key.code[UPPER_CASE] = '[';
	cp1251_to_utf8(key.name[LOWER_CASE], "[");
	cp1251_to_utf8(key.name[UPPER_CASE], "[");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// ]
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ']';
	key.code[UPPER_CASE] = ']';
	cp1251_to_utf8(key.name[LOWER_CASE], "]");
	cp1251_to_utf8(key.name[UPPER_CASE], "]");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// delete
	key.pos = curPos;
	key.symPos = ImVec2(0, 0);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_BACKSPACE;
	key.id++;
	it = row->begin();
	row->insert(it, key);

	// 4-� ������
	row = &m_Rows[LAYOUT_NUM][3];
	curPos.x = 0;
	curPos.y += m_fKeySizeY;

	key.type = KEY_DEFAULT;

	// slash (/)
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '/';
	key.code[UPPER_CASE] = '/';
	cp1251_to_utf8(key.name[LOWER_CASE], "/");
	cp1251_to_utf8(key.name[UPPER_CASE], "/");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// comma (,)
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = ',';
	key.code[UPPER_CASE] = ',';
	cp1251_to_utf8(key.name[LOWER_CASE], ",");
	cp1251_to_utf8(key.name[UPPER_CASE], ",");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// switch language
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.2, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SWITCH;
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// Space
	key.pos = curPos;
	key.symPos = ImVec2(0, 0);
	key.width = defWidth * 4;
	key.code[LOWER_CASE] = ' ';
	key.code[UPPER_CASE] = ' ';
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SPACE;
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	key.type = KEY_DEFAULT;

	// ?
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '?';
	key.code[UPPER_CASE] = '?';
	cp1251_to_utf8(key.name[LOWER_CASE], "?");
	cp1251_to_utf8(key.name[UPPER_CASE], "?");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// !
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = '!';
	key.code[UPPER_CASE] = '!';
	cp1251_to_utf8(key.name[LOWER_CASE], "!");
	cp1251_to_utf8(key.name[UPPER_CASE], "!");
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	// Send
	key.pos = curPos;
	key.symPos = ImVec2(curPos.x + defWidth * 0.4, curPos.y + m_fKeySizeY * 0.2);
	key.width = defWidth;
	key.code[LOWER_CASE] = 0;
	key.code[UPPER_CASE] = 0;
	key.name[LOWER_CASE][0] = 0;
	key.name[UPPER_CASE][0] = 0;
	key.type = KEY_SEND;
	key.id++;
	it = row->begin();
	row->insert(it, key);
	curPos.x += key.width;

	return;
}

void CKeyBoard::Flush()
{
	if (!m_sInput.length()) return;

	m_sInput.clear();
	m_iInputOffset = 0;
	memset(m_utf8Input, 0, sizeof(m_utf8Input) - 1);
}

void CKeyBoard::EnableNewKeyboard()
{
	m_bNewKeyboard = true;
}

void CKeyBoard::EnableOldKeyboard()
{
	m_bNewKeyboard = false;
}

bool CKeyBoard::IsNewKeyboard()
{
	return m_bNewKeyboard;
}

void CKeyBoard::ProcessInputCommands()
{
	std::string* pStr = nullptr;
	while (pStr = bufferedStrings.ReadLock())
	{
		if (m_pHandler)
		{
			m_pHandler(pStr->c_str());
		}
		Close();

		bufferedStrings.ReadUnlock();
	}
}

void CKeyBoard::OnNewKeyboardInput(JNIEnv* pEnv, jobject thiz, jbyteArray str)
{
	if (!str)
	{
		return;
	}
	jboolean isCopy = true;

	jbyte* pMsg = pEnv->GetByteArrayElements(str, &isCopy);
	jsize length = pEnv->GetArrayLength(str);

	std::string szStr((char*)pMsg, length);

	std::string* toWrite = bufferedStrings.WriteLock();

	*toWrite = szStr;

	bufferedStrings.WriteUnlock();

	pEnv->ReleaseByteArrayElements(str, pMsg, JNI_ABORT);
}
