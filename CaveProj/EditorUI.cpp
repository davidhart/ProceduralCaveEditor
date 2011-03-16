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
	_skin->Init("DefaultSkin.png");

	_canvas = new Gwen::Controls::Canvas(_skin);
	_canvas->SetSize(renderWindow.GetSize().x, renderWindow.GetSize().y);
	// TODO: make menu functional
	Gwen::Controls::MenuStrip* m = new Gwen::Controls::MenuStrip(_canvas);
	Gwen::Controls::Menu* fileMenu = m->AddItem("File")->GetMenu();
	fileMenu->AddItem("New");
	fileMenu->AddItem("Open")->onPress.Add(this, &EditorUI::onOpenMenuItem);;
	fileMenu->AddDivider();
	fileMenu->AddItem("Save")->onPress.Add(this, &EditorUI::onSaveMenuItem);
	fileMenu->AddItem("Save As")->onPress.Add(this, &EditorUI::onSaveAsMenuItem);
	fileMenu->AddDivider();
	fileMenu->AddItem("Quit")->onPress.Add(this, &EditorUI::onQuitMenuItem);

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
	if (!_updatingProperties)
	{
		Vector3f p;

		p.x = _lightXPosition->GetFloatFromText();
		p.y = _lightYPosition->GetFloatFromText();
		p.z = _lightZPosition->GetFloatFromText();

		_environment->SetLightPosition(_editor->SelectedLight(), p);
		_environment->SetLightFalloff(_editor->SelectedLight(), _lightFalloff->GetFloatFromText());
		_environment->SetLightSize(_editor->SelectedLight(), _lightSize->GetFloatFromText());
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
	_octaveList->GetTable()->RemoveAllChildren();
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
	_shapeList->GetTable()->RemoveAllChildren();
	_shapeRows.clear();
	for (int i = 0; i < _environment->NumShapes(); ++i)
	{
		std::stringstream ss;
		ss << "Metaball " << i + 1;
		_shapeRows.push_back(_shapeList->AddItem(ss.str()));
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
					_editor->ResetCamera();

					PopulateLightList();
					PopulateOctaveList();
					PopulateShapeList();
				}
			}
		}
	}
}

void EditorUI::onQuitMenuItem(Gwen::Controls::Base* from)
{
	_renderWindow.Close();
}