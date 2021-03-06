#include "EditorUI.h"
#include "Editor.h"
#include "Util.h"
#include "RenderWindow.h"

#include <Gwen/Controls/WindowControl.h>
#include <Gwen/Controls/DockedTabControl.h>
#include <Gwen/Controls/PanelListPanel.h>
#include <Gwen/Controls/DockBase.h>
#include <Gwen/Controls/MenuStrip.h>
#include <Gwen/Controls/ComboBox.h>
#include <sstream>
#include <Shobjidl.h>

EditorUI::EditorUI(RenderWindow& renderWindow) : 
	_renderer(NULL),
	_canvas(NULL),
	_skin(NULL),
	_dockBase(NULL),
	_editor(NULL),
	_environment(NULL),
	_lightsList(NULL),
	_lightXPosition(NULL),
	_lightYPosition(NULL),
	_lightZPosition(NULL),
	_lightFalloff(NULL),
	_lightSize(NULL),
	_octaveList(NULL),
	_octaveXScale(NULL),
	_octaveYScale(NULL),
	_octaveZScale(NULL),
	_octaveAmplitude(NULL),
	_selectedOctave(-1),
	_updatingProperties(false),
	_renderWindow(renderWindow)
{
}

void EditorUI::Load(RenderWindow& renderWindow)
{
	_renderer = new Gwen::Renderer::DirectX10(renderWindow.GetDevice());
	_skin = new Gwen::Skin::TexturedBase();
	_skin->SetRender(_renderer);
	_skin->Init("Assets/DefaultSkin.png");

	_canvas = new Gwen::Controls::Canvas(_skin);
	_canvas->SetSize(renderWindow.GetSize().x, renderWindow.GetSize().y);

	// Setup menu
	Gwen::Controls::MenuStrip* m = new Gwen::Controls::MenuStrip(_canvas);
	Gwen::Controls::Menu* fileMenu = m->AddItem("File")->GetMenu();
	fileMenu->AddItem("New")->onPress.Add(this, &EditorUI::onNewMenuItem);
	fileMenu->AddItem("Open")->onPress.Add(this, &EditorUI::onOpenMenuItem);
	fileMenu->AddDivider();
	fileMenu->AddItem("Save")->onPress.Add(this, &EditorUI::onSaveMenuItem);
	fileMenu->AddItem("Save As")->onPress.Add(this, &EditorUI::onSaveAsMenuItem);
	fileMenu->AddDivider();
	fileMenu->AddItem("Quit")->onPress.Add(this, &EditorUI::onQuitMenuItem);

	Gwen::Controls::Menu* viewMenu = m->AddItem("View")->GetMenu();
	viewMenu->AddItem("Preview")->onPress.Add(this, &EditorUI::onPreviewMenuItem);
	viewMenu->AddItem("Reset View")->onPress.Add(this, &EditorUI::onResetViewMenuItem);

	// Setup dock sidebar
	m->Dock(Gwen::Pos::Top);
	m->SetSize(100, 20);

	_dockBase = new Gwen::Controls::DockBase(_canvas);
	_dockBase->Dock(Gwen::Pos::Fill);

	CreateShapePage();
	CreateNoisePage();
	CreateLightingPage();
	CreateObjectsPage();

	_inputHelper.Initialize(_canvas);
}

void EditorUI::Unload()
{
	delete _canvas;
	delete _skin;
	delete _renderer;
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

	int yPos = 0;

	_shapeList = new Gwen::Controls::ListBox(s);
	_shapeList->SetBounds(0,yPos, 116, 106);
	_shapeList->onRowSelected.Add(this, &EditorUI::onShapeSelected);

	PopulateShapeList();

	Gwen::Controls::Button* addButton = new Gwen::Controls::Button(s);
	addButton->SetBounds(122,yPos,50, 50);
	addButton->SetText("+");
	addButton->onPress.Add(this, &EditorUI::onAddShape);

	Gwen::Controls::Button* removeButton = new Gwen::Controls::Button(s);
	removeButton->SetBounds(122, yPos+56, 50, 50);
	removeButton->SetText("-");
	removeButton->onPress.Add(this, &EditorUI::onRemoveShape);

	yPos += 118;
	Gwen::Controls::ComboBox* shapeCombo = new Gwen::Controls::ComboBox(s);
	shapeCombo->SetBounds(60, yPos, 112, 20);
	shapeCombo->AddItem(L"Metaball");

	Gwen::Controls::Label* shapeLabel = new Gwen::Controls::Label(s);
	shapeLabel->SetBounds(0, yPos, 54, 20);
	shapeLabel->SetText("Shape:");
	shapeLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Position controls
	yPos += 32;
	Gwen::Controls::Label* positionLabel = new Gwen::Controls::Label(s);
	positionLabel->SetBounds(16, yPos, 156, 20);
	positionLabel->SetText("Position");
	positionLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	yPos += 20;
	_shapeXPosition = new Gwen::Controls::TextBoxNumeric(s);
	_shapeXPosition->SetBounds(60, yPos, 112, 20);
	_shapeXPosition->SetText("0");
	_shapeXPosition->onTextChanged.Add(this, &EditorUI::onShapePropertiesChange);

	Gwen::Controls::Label* positionXLabel = new Gwen::Controls::Label(s);
	positionXLabel->SetBounds(0, yPos, 54, 20);
	positionXLabel->SetText("x:");
	positionXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	yPos += 26;
	_shapeYPosition = new Gwen::Controls::TextBoxNumeric(s);
	_shapeYPosition->SetBounds(60, yPos, 112, 20);
	_shapeYPosition->SetText("0");
	_shapeYPosition->onTextChanged.Add(this, &EditorUI::onShapePropertiesChange);

	Gwen::Controls::Label* positionYLabel = new Gwen::Controls::Label(s);
	positionYLabel->SetBounds(0, yPos, 54, 20);
	positionYLabel->SetText("y:");
	positionYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	yPos += 26;
	_shapeZPosition = new Gwen::Controls::TextBoxNumeric(s);
	_shapeZPosition->SetBounds(60, yPos, 112, 20);
	_shapeZPosition->SetText("0");
	_shapeZPosition->onTextChanged.Add(this, &EditorUI::onShapePropertiesChange);

	Gwen::Controls::Label* positionZLabel = new Gwen::Controls::Label(s);
	positionZLabel->SetBounds(0, yPos, 54, 20);
	positionZLabel->SetText("z:");
	positionZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Scale controls
	yPos += 32;
	Gwen::Controls::Label* scaleLabel = new Gwen::Controls::Label(s);
	scaleLabel->SetBounds(16, yPos, 156, 20);
	scaleLabel->SetText("Scale");
	scaleLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	yPos += 20;
	_shapeXScale = new Gwen::Controls::TextBoxNumeric(s);
	_shapeXScale->SetBounds(60, yPos, 112, 20);
	_shapeXScale->SetText("0");
	_shapeXScale->onTextChanged.Add(this, &EditorUI::onShapePropertiesChange);

	Gwen::Controls::Label* scaleXLabel = new Gwen::Controls::Label(s);
	scaleXLabel->SetBounds(0, yPos, 54, 20);
	scaleXLabel->SetText("x:");
	scaleXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	yPos += 26;
	_shapeYScale = new Gwen::Controls::TextBoxNumeric(s);
	_shapeYScale->SetBounds(60, yPos, 112, 20);
	_shapeYScale->SetText("0");
	_shapeYScale->onTextChanged.Add(this, &EditorUI::onShapePropertiesChange);

	Gwen::Controls::Label* scaleYLabel = new Gwen::Controls::Label(s);
	scaleYLabel->SetBounds(0, yPos, 54, 20);
	scaleYLabel->SetText("y:");
	scaleYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	yPos += 26;
	_shapeZScale = new Gwen::Controls::TextBoxNumeric(s);
	_shapeZScale->SetBounds(60, yPos, 112, 20);
	_shapeZScale->SetText("0");
	_shapeZScale->onTextChanged.Add(this, &EditorUI::onShapePropertiesChange);

	Gwen::Controls::Label* scaleZLabel = new Gwen::Controls::Label(s);
	scaleZLabel->SetBounds(0, yPos, 54, 20);
	scaleZLabel->SetText("z:");
	scaleZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);
}

void EditorUI::CreateNoisePage()
{
	Gwen::Controls::ScrollControl* s = new Gwen::Controls::ScrollControl(_canvas);
	_dockBase->GetRight()->GetTabControl()->AddPage(L"Noise", s);
	s->SetAutoHideBars(true);

	_octaveList = new Gwen::Controls::ListBox(s);
	_octaveList->SetBounds(0,0, 116, 106);
	_octaveList->onRowSelected.Add(this, &EditorUI::onOctaveSelected);

	PopulateOctaveList();

	Gwen::Controls::Button* addButton = new Gwen::Controls::Button(s);
	addButton->SetBounds(122,0,50, 50);
	addButton->SetText("+");
	addButton->onPress.Add(this, &EditorUI::onAddOctave);

	Gwen::Controls::Button* removeButton = new Gwen::Controls::Button(s);
	removeButton->SetBounds(122, 56, 50, 50);
	removeButton->SetText("-");
	removeButton->onPress.Add(this, &EditorUI::onRemoveOctave);

	_octaveAmplitude = new Gwen::Controls::TextBoxNumeric(s);
	_octaveAmplitude->SetBounds(60, 118, 112, 20);
	_octaveAmplitude->SetText("0");
	_octaveAmplitude->onTextChanged.Add(this, &EditorUI::onNoisePropertiesChange);
	
	Gwen::Controls::Label* amplitudeLabel = new Gwen::Controls::Label(s);
	amplitudeLabel->SetBounds(0, 118, 54, 20);
	amplitudeLabel->SetText("Amplitude:");
	amplitudeLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Scale controls
	Gwen::Controls::Label* scaleLabel = new Gwen::Controls::Label(s);
	scaleLabel->SetBounds(16, 150, 156, 20);
	scaleLabel->SetText("Scale");
	scaleLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	_octaveXScale = new Gwen::Controls::TextBoxNumeric(s);
	_octaveXScale->SetBounds(60, 176, 112, 20);
	_octaveXScale->SetText("0");
	_octaveXScale->onTextChanged.Add(this, &EditorUI::onNoisePropertiesChange);

	Gwen::Controls::Label* scaleXLabel = new Gwen::Controls::Label(s);
	scaleXLabel->SetBounds(0, 176, 54, 20);
	scaleXLabel->SetText("x:");
	scaleXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	_octaveYScale = new Gwen::Controls::TextBoxNumeric(s);
	_octaveYScale->SetBounds(60, 202, 112, 20);
	_octaveYScale->SetText("0");
	_octaveYScale->onTextChanged.Add(this, &EditorUI::onNoisePropertiesChange);

	Gwen::Controls::Label* scaleYLabel = new Gwen::Controls::Label(s);
	scaleYLabel->SetBounds(0, 202, 54, 20);
	scaleYLabel->SetText("y:");
	scaleYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	_octaveZScale = new Gwen::Controls::TextBoxNumeric(s);
	_octaveZScale->SetBounds(60, 228, 112, 20);
	_octaveZScale->SetText("0");
	_octaveZScale->onTextChanged.Add(this, &EditorUI::onNoisePropertiesChange);

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
	_lightColor = new Gwen::Controls::HSVColorPicker(s);
	_lightColor->SetBounds(0, 138, 176, 150);
	_lightColor->onColorChanged.Add(this, &EditorUI::onLightPropertiesChange);

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
}

void EditorUI::CreateObjectsPage()
{
	Gwen::Controls::ScrollControl* s = new Gwen::Controls::ScrollControl(_canvas);
	_dockBase->GetRight()->GetTabControl()->AddPage(L"Objects", s);
	s->SetAutoHideBars(true);

	int yPos = 0;

	_objectsList = new Gwen::Controls::ListBox(s);
	_objectsList->SetBounds(0,yPos, 116, 106);
	_objectsList->onRowSelected.Add(this, &EditorUI::onObjectSelected);

	PopulateObjectList();

	Gwen::Controls::Button* addButton = new Gwen::Controls::Button(s);
	addButton->SetBounds(122,yPos,50, 50);
	addButton->SetText("+");
	addButton->onPress.Add(this, &EditorUI::onAddObject);

	Gwen::Controls::Button* removeButton = new Gwen::Controls::Button(s);
	removeButton->SetBounds(122, yPos+56, 50, 50);
	removeButton->SetText("-");
	removeButton->onPress.Add(this, &EditorUI::onRemoveObject);

	yPos += 118;
	Gwen::Controls::ComboBox* objectCombo = new Gwen::Controls::ComboBox(s);
	objectCombo->SetBounds(60, yPos, 112, 20);
	objectCombo->AddItem(L"Chest");

	Gwen::Controls::Label* objectLabel = new Gwen::Controls::Label(s);
	objectLabel->SetBounds(0, yPos, 54, 20);
	objectLabel->SetText("Type:");
	objectLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	// Position controls
	yPos += 32;
	Gwen::Controls::Label* positionLabel = new Gwen::Controls::Label(s);
	positionLabel->SetBounds(16, yPos, 156, 20);
	positionLabel->SetText("Position");
	positionLabel->SetAlignment(Gwen::Pos::Left | Gwen::Pos::CenterV);

	yPos += 20;
	_objectXPosition = new Gwen::Controls::TextBoxNumeric(s);
	_objectXPosition->SetBounds(60, yPos, 112, 20);
	_objectXPosition->SetText("0");
	_objectXPosition->onTextChanged.Add(this, &EditorUI::onObjectPropertiesChange);

	Gwen::Controls::Label* positionXLabel = new Gwen::Controls::Label(s);
	positionXLabel->SetBounds(0, yPos, 54, 20);
	positionXLabel->SetText("x:");
	positionXLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	yPos += 26;
	_objectYPosition = new Gwen::Controls::TextBoxNumeric(s);
	_objectYPosition->SetBounds(60, yPos, 112, 20);
	_objectYPosition->SetText("0");
	_objectYPosition->onTextChanged.Add(this, &EditorUI::onObjectPropertiesChange);

	Gwen::Controls::Label* positionYLabel = new Gwen::Controls::Label(s);
	positionYLabel->SetBounds(0, yPos, 54, 20);
	positionYLabel->SetText("y:");
	positionYLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);

	yPos += 26;
	_objectZPosition = new Gwen::Controls::TextBoxNumeric(s);
	_objectZPosition->SetBounds(60, yPos, 112, 20);
	_objectZPosition->SetText("0");
	_objectZPosition->onTextChanged.Add(this, &EditorUI::onObjectPropertiesChange);

	Gwen::Controls::Label* positionZLabel = new Gwen::Controls::Label(s);
	positionZLabel->SetBounds(0, yPos, 54, 20);
	positionZLabel->SetText("z:");
	positionZLabel->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);
}

void EditorUI::PopulateLightList()
{
	_lightsList->Clear();
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
	if (_editor->SelectedLight() >= 0)
	{
		_environment->RemoveLight(_editor->SelectedLight());
		_editor->DeselectLight();

		PopulateLightList();
		UpdateLightProperties(_editor->SelectedLight());
	}
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
	UpdateLightProperties(i);
}

void EditorUI::onLightPropertiesChange(Gwen::Controls::Base* from)
{
	if (!_updatingProperties)
	{
		if (_editor->SelectedLight() < 0)
			return;

		Vector3f p;

		p.x = _lightXPosition->GetFloatFromText();
		p.y = _lightYPosition->GetFloatFromText();
		p.z = _lightZPosition->GetFloatFromText();

		_environment->SetLightPosition(_editor->SelectedLight(), p);
		_environment->SetLightFalloff(_editor->SelectedLight(), _lightFalloff->GetFloatFromText());
		_environment->SetLightSize(_editor->SelectedLight(), _lightSize->GetFloatFromText());

		_environment->SetLightColor(_editor->SelectedLight(), COLOR_ARGB(255, _lightColor->GetColor().r, _lightColor->GetColor().g, _lightColor->GetColor().b));
	}
}

void EditorUI::SelectLight(int light)
{
	_lightsList->UnselectAll();

	if (light >= 0)
		_lightRows[light]->SetSelected(true);

	UpdateLightProperties(light);
}

void EditorUI::UpdateLightProperties(int light)
{
	if (light < 0)
		return;
	
	_updatingProperties = true;

	std::stringstream sX;
	sX << _environment->GetLightPosition(light).x;
	_lightXPosition->SetText(sX.str());

	std::stringstream sY;
	sY << _environment->GetLightPosition(light).y;
	_lightYPosition->SetText(sY.str());

	std::stringstream sZ;
	sZ << _environment->GetLightPosition(light).z;
	_lightZPosition->SetText(sZ.str());

	std::stringstream sF;
	sF << _environment->GetLightFalloff(light);
	_lightFalloff->SetText(sF.str());

	std::stringstream sS;
	sS << _environment->GetLightSize(light);
	_lightSize->SetText(sS.str());

	_lightColor->SetColor(Gwen::Color(Util::GetR(_environment->GetLightColor(light)),
		Util::GetG(_environment->GetLightColor(light)),
		Util::GetB(_environment->GetLightColor(light)),
		255));

	_updatingProperties = false;
}

void EditorUI::onAddOctave(Gwen::Controls::Base* from)
{
	int o = _environment->AddOctave();
	_environment->Rebuild();

	PopulateOctaveList();

	SelectOctave(o);
}

void EditorUI::onRemoveOctave(Gwen::Controls::Base* from)
{
	if (_selectedOctave >= 0)
	{
		_environment->RemoveOctave(_selectedOctave);
		_environment->Rebuild();

		PopulateOctaveList();

		_selectedOctave = -1;
	}
}

void EditorUI::SelectOctave(int octave)
{
	_octaveList->UnselectAll();
	
	if (octave >= 0)
		_octaveRows[octave]->SetSelected(true);

	UpdateNoiseProperties(octave);

	_selectedOctave = octave;
}

void EditorUI::onOctaveSelected(Gwen::Controls::Base* from)
{
	Gwen::Controls::Layout::TableRow* selected = _octaveList->GetSelectedRow();

	int i = 0;
	for (; i < (int)_octaveRows.size(); ++i)
	{
		if (_octaveRows[i] == selected)
			break;
	}

	if (i == _octaveRows.size())
		return; // error, selected row wasn't in rows?

	_selectedOctave = i;
	UpdateNoiseProperties(_selectedOctave);
}

void EditorUI::UpdateNoiseProperties(int octave)
{
	if (octave >= 0)
	{
		_updatingProperties = true;
		std::stringstream sX;
		sX << _environment->GetOctaveScale(octave).x;
		_octaveXScale->SetText(sX.str());

		std::stringstream sY;
		sY << _environment->GetOctaveScale(octave).y;
		_octaveYScale->SetText(sY.str());

		std::stringstream sZ;
		sZ << _environment->GetOctaveScale(octave).z;
		_octaveZScale->SetText(sZ.str());

		std::stringstream sA;
		sA << _environment->GetOctaveAmplitude(octave);
		_octaveAmplitude->SetText(sA.str());
		_updatingProperties = false;
	}
}

void EditorUI::PopulateOctaveList()
{
	_octaveList->Clear();
	_octaveRows.clear();
	for (int i = 0; i < _environment->NumOctaves(); ++i)
	{
		std::stringstream ss;
		ss << "Octave " << i + 1;
		_octaveRows.push_back(_octaveList->AddItem(ss.str()));
	}
}

void EditorUI::onNoisePropertiesChange(Gwen::Controls::Base* from)
{
	if (_selectedOctave >= 0)
	{
		if (!_updatingProperties)
		{
			Vector3f s;

			s.x = _octaveXScale->GetFloatFromText();
			s.y = _octaveYScale->GetFloatFromText();
			s.z = _octaveZScale->GetFloatFromText();

			_environment->SetOctaveScale(_selectedOctave, s);
			_environment->SetOctaveAmplitude(_selectedOctave, _octaveAmplitude->GetFloatFromText());

			_environment->Rebuild();
		}
	}
}

void EditorUI::onAddShape(Gwen::Controls::Base* from)
{
	int shape = _environment->AddShape();

	PopulateShapeList();

	SelectShape(shape);

	_environment->Rebuild();
}

void EditorUI::onRemoveShape(Gwen::Controls::Base* from)
{
	if (_editor->SelectedShape() < 0)
		return;

	_environment->RemoveShape(_editor->SelectedShape());
	_editor->DeselectShape();

	PopulateShapeList();

	_environment->Rebuild();
}

void EditorUI::SelectShape(int shape)
{
	_shapeList->UnselectAll();
	
	if (shape >= 0)
		_shapeRows[shape]->SetSelected(true);

	UpdateShapeProperties(shape);
}

void EditorUI::onShapeSelected(Gwen::Controls::Base* from)
{
	Gwen::Controls::Layout::TableRow* selected = _shapeList->GetSelectedRow();

	int i = 0;
	for (; i < (int)_shapeRows.size(); ++i)
	{
		if (_shapeRows[i] == selected)
			break;
	}

	if (i == _shapeRows.size())
		return; // error, selected row wasn't in rows?

	_editor->SelectShape(i);
	UpdateShapeProperties(i);
}

void EditorUI::UpdateShapeProperties(int shape)
{
	if (shape >= 0)
	{
		_updatingProperties = true;
		std::stringstream pX;
		pX << _environment->GetShapePosition(shape).x;
		_shapeXPosition->SetText(pX.str());

		std::stringstream pY;
		pY << _environment->GetShapePosition(shape).y;
		_shapeYPosition->SetText(pY.str());

		std::stringstream pZ;
		pZ << _environment->GetShapePosition(shape).z;
		_shapeZPosition->SetText(pZ.str());

		std::stringstream sX;
		sX << _environment->GetShapeScale(shape).x;
		_shapeXScale->SetText(sX.str());

		std::stringstream sY;
		sY << _environment->GetShapeScale(shape).y;
		_shapeYScale->SetText(sY.str());

		std::stringstream sZ;
		sZ << _environment->GetShapeScale(shape).z;
		_shapeZScale->SetText(sZ.str());

		_updatingProperties = false;
	}
}

void EditorUI::onShapePropertiesChange(Gwen::Controls::Base* from)
{
	if (!_updatingProperties)
	{
		if (_editor->SelectedShape() < 0)
			return;

		Vector3f p;

		p.x = _shapeXPosition->GetFloatFromText();
		p.y = _shapeYPosition->GetFloatFromText();
		p.z = _shapeZPosition->GetFloatFromText();

		_environment->SetShapePosition(_editor->SelectedShape(), p);

		Vector3f s;
		s.x = _shapeXScale->GetFloatFromText();
		s.y = _shapeYScale->GetFloatFromText();
		s.z = _shapeZScale->GetFloatFromText();

		_environment->SetShapeScale(_editor->SelectedShape(), s);

		_environment->Rebuild();
	}
}

void EditorUI::PopulateShapeList()
{
	_shapeList->Clear();
	_shapeRows.clear();
	for (int i = 0; i < _environment->NumShapes(); ++i)
	{
		std::stringstream ss;
		ss << "Metaball " << i + 1;
		_shapeRows.push_back(_shapeList->AddItem(ss.str()));
	}
}

void EditorUI::PopulateObjectList()
{
	_objectsList->Clear();
	_objectRows.clear();
	for (int i = 0; i < _environment->NumChests(); ++i)
	{
		std::stringstream ss;
		ss << "Chest " << i + 1;
		_objectRows.push_back(_objectsList->AddItem(ss.str()));
	}
}

void EditorUI::onRemoveObject(Gwen::Controls::Base* from)
{
	if (_editor->SelectedObject() >= 0)
	{
		_environment->RemoveChest(_editor->SelectedObject());
		_editor->DeselectObject();

		PopulateObjectList();
		UpdateObjectProperties(_editor->SelectedObject());
	}
}

void EditorUI::onAddObject(Gwen::Controls::Base* from)
{
	int object = _environment->AddChest();

	PopulateObjectList();

	SelectObject(object);
	_editor->SelectObject(object);
}

void EditorUI::onObjectPropertiesChange(Gwen::Controls::Base* from)
{
	if (!_updatingProperties)
	{
		if (_editor->SelectedObject() < 0)
			return;

		Vector3f p;

		p.x = _objectXPosition->GetFloatFromText();
		p.y = _objectYPosition->GetFloatFromText();
		p.z = _objectZPosition->GetFloatFromText();

		_environment->SetChestPosition(_editor->SelectedObject(), p);
	}
}

void EditorUI::SelectObject(int object)
{
	_objectsList->UnselectAll();

	if (object >= 0)
		_objectRows[object]->SetSelected(true);

	UpdateObjectProperties(object);
}

void EditorUI::onObjectSelected(Gwen::Controls::Base* from)
{
	Gwen::Controls::Layout::TableRow* selected = _objectsList->GetSelectedRow();

	int i = 0;
	for (; i < (int)_objectRows.size(); ++i)
	{
		if (_objectRows[i] == selected)
			break;
	}

	if (i == _objectRows.size())
		return;

	_editor->SelectObject(i);
	UpdateObjectProperties(i);
}

void EditorUI::UpdateObjectProperties(int object)
{
	if (object >= 0)
	{
		_updatingProperties = true;
		std::stringstream pX;
		pX << _environment->GetChestPosition(object).x;
		_objectXPosition->SetText(pX.str());

		std::stringstream pY;
		pY << _environment->GetChestPosition(object).y;
		_objectYPosition->SetText(pY.str());

		std::stringstream pZ;
		pZ << _environment->GetChestPosition(object).z;
		_objectZPosition->SetText(pZ.str());

		_updatingProperties = false;
	}
}

void EditorUI::onSaveMenuItem(Gwen::Controls::Base* from)
{
	if (!_currentfilename.empty())
	{
		_environment->Save(_currentfilename);
	}
	else
	{
		onSaveAsMenuItem(from);
	}
}

void EditorUI::onSaveAsMenuItem(Gwen::Controls::Base* from)
{
	IFileDialog *pfd;

	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_PPV_ARGS(&pfd));

	if (SUCCEEDED(hr))
	{
		COMDLG_FILTERSPEC filetypes[] = { L"Cave", L"*.cave" };
		pfd->SetFileTypes(1, filetypes);
		pfd->SetDefaultExtension(L"cave");
		hr = pfd->Show(_renderWindow.GetWnd());

		if (SUCCEEDED(hr))
		{
			IShellItem *psiResult;
			hr = pfd->GetResult(&psiResult);

			if (SUCCEEDED(hr))
			{
				LPWSTR filename;
				psiResult->GetDisplayName(SIGDN_FILESYSPATH, &filename);
				std::wstring s(filename);

				CoTaskMemFree(filename);
				psiResult->Release();

				if (_environment->Save(s))
				{
					_currentfilename = s;
				}
				else
				{
					MessageBox(_renderWindow.GetWnd(), "File could not be saved", "Error", MB_OK);
				}
			}
		}
		pfd->Release();
	}
}

void EditorUI::onOpenMenuItem(Gwen::Controls::Base* from)
{
	IFileDialog *pfd;
	
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_PPV_ARGS(&pfd));

	if (SUCCEEDED(hr))
	{
		COMDLG_FILTERSPEC filetypes[] = { L"Cave", L"*.cave" };
		pfd->SetFileTypes(1, filetypes);
		pfd->SetDefaultExtension(L"cave");
		hr = pfd->Show(_renderWindow.GetWnd());

		if (SUCCEEDED(hr))
		{
			IShellItem *psiResult;
			hr = pfd->GetResult(&psiResult);

			if (SUCCEEDED(hr))
			{
				LPWSTR filename;
				psiResult->GetDisplayName(SIGDN_FILESYSPATH, &filename);
				std::wstring s(filename);

				CoTaskMemFree(filename);
				psiResult->Release();

				if (_environment->Open(s))
				{
					_currentfilename = s;
					_editor->DeselectLight();
					_editor->DeselectShape();
					_editor->DeselectObject();
					_editor->ResetCamera();

					PopulateLightList();
					PopulateOctaveList();
					PopulateShapeList();
					PopulateObjectList();
				}
				else
				{
					MessageBox(_renderWindow.GetWnd(), "Invalid or incompatible file, the file is possibly intended for a different version of the application or corrupt", "Error", MB_OK);
				}
			}
		}
	}
}

void EditorUI::onQuitMenuItem(Gwen::Controls::Base* from)
{
	_renderWindow.Close();
}

void EditorUI::onNewMenuItem(Gwen::Controls::Base* from)
{
	_environment->New();
	_currentfilename.clear();
	_editor->DeselectLight();
	_editor->DeselectShape();
	_editor->DeselectObject();
	_editor->ResetCamera();
	_selectedOctave = -1;
	PopulateLightList();
	PopulateOctaveList();
	PopulateShapeList();
	PopulateObjectList();

	UpdateLightProperties(_editor->SelectedLight());
	UpdateShapeProperties(_editor->SelectedShape());
	UpdateNoiseProperties(_selectedOctave);
	UpdateObjectProperties(_editor->SelectedObject());
}

void EditorUI::onPreviewMenuItem(Gwen::Controls::Base* from)
{
	_editor->Preview(true);
}

void EditorUI::onResetViewMenuItem(Gwen::Controls::Base* from)
{
	_editor->ResetCamera();
}