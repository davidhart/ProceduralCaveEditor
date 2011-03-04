#include "EditorUI.h"
#include "Editor.h"
#include "RenderWindow.h"

#include <Gwen/Controls/WindowControl.h>
#include <Gwen/Controls/DockedTabControl.h>
#include <Gwen/Controls/PanelListPanel.h>
#include <Gwen/Controls/DockBase.h>
#include <Gwen/Controls/MenuStrip.h>
#include <Gwen/Controls/ComboBox.h>
#include <Gwen/Controls/HSVColorPicker.h>
#include <sstream>

EditorUI::EditorUI() : 
	_renderer(NULL),
	_canvas(NULL),
	_dockBase(NULL),
	_editor(NULL),
	_environment(NULL),
	_lightXPosition(NULL),
	_lightYPosition(NULL),
	_lightZPosition(NULL)
{
}

void EditorUI::Load(RenderWindow& renderWindow)
{
	_renderer = new Gwen::Renderer::DirectX10(renderWindow.GetDevice());
	_skin.SetRender(_renderer);
	_skin.Init("DefaultSkin.png");

	_canvas = new Gwen::Controls::Canvas(&_skin);
	_canvas->SetSize(renderWindow.GetSize().x, renderWindow.GetSize().y);
	Gwen::Controls::MenuStrip* m = new Gwen::Controls::MenuStrip(_canvas);
	Gwen::Controls::Menu* fileMenu = m->AddItem("File")->GetMenu();
	fileMenu->AddItem("New");
	fileMenu->AddItem("Open");
	fileMenu->AddDivider();
	fileMenu->AddItem("Save");
	fileMenu->AddItem("Save As");
	fileMenu->AddDivider();
	fileMenu->AddItem("Quit");

	Gwen::Controls::Menu* viewMenu = m->AddItem("View")->GetMenu();
	viewMenu->AddItem("Editor");

	m->Dock(Gwen::Pos::Top);
	m->SetSize(100, 20);

	_dockBase = new Gwen::Controls::DockBase(_canvas);
	_dockBase->Dock(Gwen::Pos::Fill);

	CreateShapePage();
	CreateNoisePage();
	CreateWaterPage();
	CreateLightingPage();
	CreateObjectsPage();

	_inputHelper.Initialize(_canvas);
}

void EditorUI::Unload()
{
	// TODO: test for memory leaks
}

void EditorUI::Draw()
{
	_canvas->RenderCanvas();
}

void EditorUI::HandleMessage(MSG msg)
{
	_inputHelper.ProcessMessage(msg);
}

void EditorUI::CreateShapePage()
{
	Gwen::Controls::ScrollControl* s = new Gwen::Controls::ScrollControl(_canvas);
	_dockBase->GetRight()->GetTabControl()->AddPage(L"Shape", s);
	s->SetAutoHideBars(true);
	Gwen::Controls::ListBox* shapeList = new Gwen::Controls::ListBox(s);
	shapeList->SetBounds(0,0, 116, 106);
	shapeList->AddItem(L"Metaball 1");
	shapeList->AddItem(L"Metaball 2");
	shapeList->AddItem(L"Metaball 3");
	shapeList->AddItem(L"Room 1");
	shapeList->AddItem(L"Cylinder 1");
	shapeList->AddItem(L"Cylinder 2");

	Gwen::Controls::Button* addButton = new Gwen::Controls::Button(s);
	addButton->SetBounds(122,0,50, 50);
	addButton->SetText("+");
	Gwen::Controls::Button* removeButton = new Gwen::Controls::Button(s);
	removeButton->SetBounds(122, 56, 50, 50);
	removeButton->SetText("-");

	Gwen::Controls::TextBox* nameBox = new Gwen::Controls::TextBox(s);
	nameBox->SetBounds(60, 118, 112, 20);
	nameBox->SetText("Metaball 1");
	
	Gwen::Controls::Label* nameLabel = new Gwen::Controls::Label(s);
	nameLabel->SetBounds(0, 118, 54, 20);
	nameLabel->SetText("Name:");
	nameLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::ComboBox* shapeCombo = new Gwen::Controls::ComboBox(s);
	shapeCombo->SetBounds(60, 144, 112, 20);
	shapeCombo->AddItem(L"Metaball");
	shapeCombo->AddItem(L"Cube");
	shapeCombo->AddItem(L"Cylinder");

	Gwen::Controls::Label* shapeLabel = new Gwen::Controls::Label(s);
	shapeLabel->SetBounds(0, 144, 54, 20);
	shapeLabel->SetText("Shape:");
	shapeLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Position controls
	Gwen::Controls::Label* positionLabel = new Gwen::Controls::Label(s);
	positionLabel->SetBounds(16, 176, 156, 20);
	positionLabel->SetText("Position");
	positionLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* positionXBox = new Gwen::Controls::TextBoxNumeric(s);
	positionXBox->SetBounds(60, 196, 112, 20);
	positionXBox->SetText("0");

	Gwen::Controls::Label* positionXLabel = new Gwen::Controls::Label(s);
	positionXLabel->SetBounds(0, 196, 54, 20);
	positionXLabel->SetText("x:");
	positionXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* positionYBox = new Gwen::Controls::TextBoxNumeric(s);
	positionYBox->SetBounds(60, 222, 112, 20);
	positionYBox->SetText("0");

	Gwen::Controls::Label* positionYLabel = new Gwen::Controls::Label(s);
	positionYLabel->SetBounds(0, 222, 54, 20);
	positionYLabel->SetText("y:");
	positionYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* positionZBox = new Gwen::Controls::TextBoxNumeric(s);
	positionZBox->SetBounds(60, 248, 112, 20);
	positionZBox->SetText("0");

	Gwen::Controls::Label* positionZLabel = new Gwen::Controls::Label(s);
	positionZLabel->SetBounds(0, 248, 54, 20);
	positionZLabel->SetText("z:");
	positionZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Rotation controls
	Gwen::Controls::Label* rotationLabel = new Gwen::Controls::Label(s);
	rotationLabel->SetBounds(16, 280, 156, 20);
	rotationLabel->SetText("Rotation");
	rotationLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* rotationXBox = new Gwen::Controls::TextBoxNumeric(s);
	rotationXBox->SetBounds(60, 306, 112, 20);
	rotationXBox->SetText("0");

	Gwen::Controls::Label* rotationXLabel = new Gwen::Controls::Label(s);
	rotationXLabel->SetBounds(0, 306, 54, 20);
	rotationXLabel->SetText("x:");
	rotationXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* rotationYBox = new Gwen::Controls::TextBoxNumeric(s);
	rotationYBox->SetBounds(60, 332, 112, 20);
	rotationYBox->SetText("0");

	Gwen::Controls::Label* rotationYLabel = new Gwen::Controls::Label(s);
	rotationYLabel->SetBounds(0, 332, 54, 20);
	rotationYLabel->SetText("y:");
	rotationYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* rotationZBox = new Gwen::Controls::TextBoxNumeric(s);
	rotationZBox->SetBounds(60, 356, 112, 20);
	rotationZBox->SetText("0");

	Gwen::Controls::Label* rotationZLabel = new Gwen::Controls::Label(s);
	rotationZLabel->SetBounds(0, 356, 54, 20);
	rotationZLabel->SetText("z:");
	rotationZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Scale controls
	Gwen::Controls::Label* scaleLabel = new Gwen::Controls::Label(s);
	scaleLabel->SetBounds(16, 390, 156, 20);
	scaleLabel->SetText("Scale");
	scaleLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* scaleXBox = new Gwen::Controls::TextBoxNumeric(s);
	scaleXBox->SetBounds(60, 416, 112, 20);
	scaleXBox->SetText("0");

	Gwen::Controls::Label* scaleXLabel = new Gwen::Controls::Label(s);
	scaleXLabel->SetBounds(0, 416, 54, 20);
	scaleXLabel->SetText("x:");
	scaleXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* scaleYBox = new Gwen::Controls::TextBoxNumeric(s);
	scaleYBox->SetBounds(60, 442, 112, 20);
	scaleYBox->SetText("0");

	Gwen::Controls::Label* scaleYLabel = new Gwen::Controls::Label(s);
	scaleYLabel->SetBounds(0, 442, 54, 20);
	scaleYLabel->SetText("y:");
	scaleYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* scaleZBox = new Gwen::Controls::TextBoxNumeric(s);
	scaleZBox->SetBounds(60, 468, 112, 20);
	scaleZBox->SetText("0");

	Gwen::Controls::Label* scaleZLabel = new Gwen::Controls::Label(s);
	scaleZLabel->SetBounds(0, 468, 54, 20);
	scaleZLabel->SetText("z:");
	scaleZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);
}

void EditorUI::CreateNoisePage()
{
	Gwen::Controls::ScrollControl* s = new Gwen::Controls::ScrollControl(_canvas);
	_dockBase->GetRight()->GetTabControl()->AddPage(L"Noise", s);
	s->SetAutoHideBars(true);

	Gwen::Controls::ListBox* octaveList = new Gwen::Controls::ListBox(s);
	octaveList->SetBounds(0,0, 116, 106);
	octaveList->AddItem(L"Octave 1");
	octaveList->AddItem(L"Octave 2");
	octaveList->AddItem(L"Octave 3");
	octaveList->AddItem(L"Octave 4");

	Gwen::Controls::Button* addButton = new Gwen::Controls::Button(s);
	addButton->SetBounds(122,0,50, 50);
	addButton->SetText("+");
	Gwen::Controls::Button* removeButton = new Gwen::Controls::Button(s);
	removeButton->SetBounds(122, 56, 50, 50);
	removeButton->SetText("-");

	Gwen::Controls::TextBoxNumeric* amplitudeBox = new Gwen::Controls::TextBoxNumeric(s);
	amplitudeBox->SetBounds(60, 118, 112, 20);
	amplitudeBox->SetText("0");
	
	Gwen::Controls::Label* amplitudeLabel = new Gwen::Controls::Label(s);
	amplitudeLabel->SetBounds(0, 118, 54, 20);
	amplitudeLabel->SetText("Amplitude:");
	amplitudeLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Scale controls
	Gwen::Controls::Label* scaleLabel = new Gwen::Controls::Label(s);
	scaleLabel->SetBounds(16, 150, 156, 20);
	scaleLabel->SetText("Scale");
	scaleLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* scaleXBox = new Gwen::Controls::TextBoxNumeric(s);
	scaleXBox->SetBounds(60, 176, 112, 20);
	scaleXBox->SetText("0");

	Gwen::Controls::Label* scaleXLabel = new Gwen::Controls::Label(s);
	scaleXLabel->SetBounds(0, 176, 54, 20);
	scaleXLabel->SetText("x:");
	scaleXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* scaleYBox = new Gwen::Controls::TextBoxNumeric(s);
	scaleYBox->SetBounds(60, 202, 112, 20);
	scaleYBox->SetText("0");

	Gwen::Controls::Label* scaleYLabel = new Gwen::Controls::Label(s);
	scaleYLabel->SetBounds(0, 202, 54, 20);
	scaleYLabel->SetText("y:");
	scaleYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* scaleZBox = new Gwen::Controls::TextBoxNumeric(s);
	scaleZBox->SetBounds(60, 228, 112, 20);
	scaleZBox->SetText("0");

	Gwen::Controls::Label* scaleZLabel = new Gwen::Controls::Label(s);
	scaleZLabel->SetBounds(0, 228, 54, 20);
	scaleZLabel->SetText("z:");
	scaleZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);
}

void EditorUI::CreateLightingPage()
{
	Gwen::Controls::ScrollControl* s = new Gwen::Controls::ScrollControl(_canvas);
	_dockBase->GetRight()->GetTabControl()->AddPage(L"Lighting", s);
	s->SetAutoHideBars(true);

	_lightsList = new Gwen::Controls::ListBox(s);
	_lightsList->SetBounds(0,0, 116, 106);
	_lightsList->onRowSelected.Add(this, &EditorUI::onLightSelected);

	PopulateLightList();

	Gwen::Controls::Button* addButton = new Gwen::Controls::Button(s);
	addButton->onUp.Add(this, &EditorUI::onAddLight);
	addButton->SetBounds(122,0,50, 50);
	addButton->SetText("+");
	Gwen::Controls::Button* removeButton = new Gwen::Controls::Button(s);
	removeButton->onUp.Add(this, &EditorUI::onRemoveLight);
	removeButton->SetBounds(122, 56, 50, 50);
	removeButton->SetText("-");

	Gwen::Controls::Label* colorLabel = new Gwen::Controls::Label(s);
	colorLabel->SetBounds(0, 118, 176, 20);
	colorLabel->SetText("Light Color: ");
	colorLabel->SetAlignment(Gwen::Pos::CenterV | Gwen::Pos::Left);
	//Gwen::Controls::HSVColorPicker* lightColor = new Gwen::Controls::HSVColorPicker(s);
	//lightColor->SetBounds(-4, 138, 176, 150);

	_lightSize = new Gwen::Controls::TextBoxNumeric(s);
	_lightSize->SetBounds(60, 286, 112, 20);
	_lightSize->SetText("0");
	_lightSize->onTextChanged.Add(this, &EditorUI::onLightPropertiesChange);
	
	Gwen::Controls::Label* sizeLabel = new Gwen::Controls::Label(s);
	sizeLabel->SetBounds(0, 286, 54, 20);
	sizeLabel->SetText("Size:");
	sizeLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	_lightFalloff = new Gwen::Controls::TextBoxNumeric(s);
	_lightFalloff->SetBounds(60, 312, 112, 20);
	_lightFalloff->SetText("0");
	_lightFalloff->onTextChanged.Add(this, &EditorUI::onLightPropertiesChange);
	
	Gwen::Controls::Label* falloffLabel = new Gwen::Controls::Label(s);
	falloffLabel->SetBounds(0, 312, 54, 20);
	falloffLabel->SetText("Falloff:");
	falloffLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Position controls
	Gwen::Controls::Label* positionLabel = new Gwen::Controls::Label(s);
	positionLabel->SetBounds(16, 344, 156, 20);
	positionLabel->SetText("Position");
	positionLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	_lightXPosition = new Gwen::Controls::TextBoxNumeric(s);
	_lightXPosition->SetBounds(60, 370, 112, 20);
	_lightXPosition->SetText("0");
	_lightXPosition->onTextChanged.Add(this, &EditorUI::onLightPropertiesChange);

	Gwen::Controls::Label* positionXLabel = new Gwen::Controls::Label(s);
	positionXLabel->SetBounds(0, 370, 54, 20);
	positionXLabel->SetText("x:");
	positionXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	_lightYPosition = new Gwen::Controls::TextBoxNumeric(s);
	_lightYPosition->SetBounds(60, 396, 112, 20);
	_lightYPosition->SetText("0");
	_lightYPosition->onTextChanged.Add(this, &EditorUI::onLightPropertiesChange);

	Gwen::Controls::Label* positionYLabel = new Gwen::Controls::Label(s);
	positionYLabel->SetBounds(0, 396, 54, 20);
	positionYLabel->SetText("y:");
	positionYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	_lightZPosition = new Gwen::Controls::TextBoxNumeric(s);
	_lightZPosition->SetBounds(60, 422, 112, 20);
	_lightZPosition->SetText("0");
	_lightZPosition->onTextChanged.Add(this, &EditorUI::onLightPropertiesChange);

	Gwen::Controls::Label* positionZLabel = new Gwen::Controls::Label(s);
	positionZLabel->SetBounds(0, 422, 54, 20);
	positionZLabel->SetText("z:");
	positionZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Target position controls
	Gwen::Controls::Label* targetLabel = new Gwen::Controls::Label(s);
	targetLabel->SetBounds(16, 454, 156, 20);
	targetLabel->SetText("Light Target");
	positionLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* targetXBox = new Gwen::Controls::TextBoxNumeric(s);
	targetXBox->SetBounds(60, 480, 112, 20);
	targetXBox->SetText("0");

	Gwen::Controls::Label* targetXLabel = new Gwen::Controls::Label(s);
	targetXLabel->SetBounds(0, 480, 54, 20);
	targetXLabel->SetText("x:");
	targetXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* targetYBox = new Gwen::Controls::TextBoxNumeric(s);
	targetYBox->SetBounds(60, 506, 112, 20);
	targetYBox->SetText("0");

	Gwen::Controls::Label* targetYLabel = new Gwen::Controls::Label(s);
	targetYLabel->SetBounds(0, 506, 54, 20);
	targetYLabel->SetText("y:");
	targetYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	Gwen::Controls::TextBoxNumeric* targetZBox = new Gwen::Controls::TextBoxNumeric(s);
	targetZBox->SetBounds(60, 532, 112, 20);
	targetZBox->SetText("0");

	Gwen::Controls::Label* targetZLabel = new Gwen::Controls::Label(s);
	targetZLabel->SetBounds(0, 532, 54, 20);
	targetZLabel->SetText("z:");
	targetZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);
}

void EditorUI::CreateWaterPage()
{
	Gwen::Controls::Button* b3 = new Gwen::Controls::Button(_canvas);
	_dockBase->GetRight()->GetTabControl()->AddPage(L"Water", b3);
}

void EditorUI::CreateObjectsPage()
{
	Gwen::Controls::Button* b5 = new Gwen::Controls::Button(_canvas);
	_dockBase->GetRight()->GetTabControl()->AddPage(L"Objects", b5);
}

void EditorUI::PopulateLightList()
{
	_lightsList->GetTable()->RemoveAllChildren();
	_lightRows.clear();
	for (int i = 0; i < _environment->NumLights(); ++i)
	{
		std::stringstream ss;
		ss << "Point " << i + 1;
		_lightRows.push_back(_lightsList->AddItem(ss.str()));
	}
}

void EditorUI::onAddLight(Gwen::Controls::Base* from)
{
	int lightid = _environment->AddLight();

	PopulateLightList();

	SelectLight(lightid);
}

void EditorUI::onRemoveLight(Gwen::Controls::Base* from)
{
	_environment->RemoveLight(_editor->SelectedLight());
	_editor->DeselectLight();

	PopulateLightList();
}

void EditorUI::onLightSelected(Gwen::Controls::Base* from)
{
	Gwen::Controls::Layout::TableRow* selected = _lightsList->GetSelectedRow();

	int i = 0;
	for (; i < (int)_lightRows.size(); ++i)
	{
		if (_lightRows[i] == selected)
			break;
	}

	if (i == _lightRows.size())
		return; // error, selected row wasn't in rows?

	_editor->SelectLight(i);
}

void EditorUI::onLightPropertiesChange(Gwen::Controls::Base* from)
{
	Vector3f p;

	p.x = _lightXPosition->GetFloatFromText();
	p.y = _lightYPosition->GetFloatFromText();
	p.z = _lightZPosition->GetFloatFromText();

	_environment->SetLightPosition(_editor->SelectedLight(), p);
	_environment->SetLightFalloff(_editor->SelectedLight(), _lightFalloff->GetFloatFromText());
	_environment->SetLightSize(_editor->SelectedLight(), _lightSize->GetFloatFromText());
	
}

void EditorUI::SelectLight(int light)
{
	for (unsigned int i = 0; i < _lightRows.size(); ++i)
	{
		_lightRows[light]->SetSelected(false);
	}

	_lightRows[light]->SetSelected(true);

	UpdateLightProperties(light);
}

void EditorUI::UpdateLightProperties(int light)
{
	std::stringstream sX;
	sX << _environment->GetLightPosition(light).x;
	_lightXPosition->SetText(sX.str(), false);

	std::stringstream sY;
	sY << _environment->GetLightPosition(light).y;
	_lightYPosition->SetText(sY.str(), false);

	std::stringstream sZ;
	sZ << _environment->GetLightPosition(light).z;
	_lightZPosition->SetText(sZ.str(), false);

	std::stringstream sF;
	sF << _environment->GetLightFalloff(light);
	_lightFalloff->SetText(sF.str(), false);

	std::stringstream sS;
	sS << _environment->GetLightSize(light);
	_lightSize->SetText(sS.str(), false);
}