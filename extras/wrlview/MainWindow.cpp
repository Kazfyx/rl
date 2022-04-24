//
// Copyright (c) 2009, Markus Rickert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#ifdef HAVE_SOSTLFILEKIT_H
#include <Inventor/annex/ForeignFiles/SoSTLFileKit.h>
#endif
#include <Inventor/errors/SoReadError.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/Qt/SoQt.h>

#if QT_VERSION >= 0x060000
#include <QOpenGLWindow>
#else
#include <QGLWidget>
#endif

#include "MainWindow.h"
#include "SoGradientBackground.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags f) :
	QMainWindow(parent, f),
	backgroundSwitch(nullptr),
	displayMenu(nullptr),
	fileMenu(nullptr),
	filename(),
	gradientBackground(),
	input(),
	manager(new QNetworkAccessManager(this)),
	offscreenRenderer(nullptr),
	origin1Switch(nullptr),
	origin1000Switch(nullptr),
	root(nullptr),
	scene(nullptr),
	supportedFileEndings(),
	viewer(nullptr),
	widget(new QWidget(this))
{
	SoQt::init(this);
	SoDB::init();
	SoGradientBackground::initClass();
	
	SoVRMLInline::setFetchURLCallBack(&MainWindow::inlineFetchUrlCallback, this);
	SoVRMLInline::setReadAsSoFile(false);
	
	QObject::connect(this->manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	
#if QT_VERSION >= 0x060000
	QSurfaceFormat format;
	format.setSamples(8);
	QSurfaceFormat::setDefaultFormat(format);
#else
	QGLFormat format;
	format.setAlpha(true);
	format.setSampleBuffers(true);
	QGLFormat::setDefaultFormat(format);
#endif
	
	this->setAcceptDrops(true);
	
	this->root = new SoSeparator();
	this->root->ref();
	
	this->viewer = new SoQtExaminerViewer(this->widget);
	this->viewer->setDecoration(false);
	this->viewer->setSceneGraph(this->root);
	this->viewer->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
	
	this->gradientBackground = new SoGradientBackground();
	this->gradientBackground->color0.setValue(0.8f, 0.8f, 0.8f);
	this->gradientBackground->color1.setValue(1.0f, 1.0f, 1.0f);
	
	this->backgroundSwitch = new SoSwitch();
	this->backgroundSwitch->whichChild = SO_SWITCH_ALL;
	this->backgroundSwitch->addChild(this->gradientBackground);
	this->root->addChild(this->backgroundSwitch);
	
	SoCoordinate3* originCoordinate3 = new SoCoordinate3();
	originCoordinate3->point.set1Value(0, SbVec3f(0, 0, 0));
	originCoordinate3->point.set1Value(1, SbVec3f(1, 0, 0));
	originCoordinate3->point.set1Value(2, SbVec3f(0, 1, 0));
	originCoordinate3->point.set1Value(3, SbVec3f(0, 0, 1));
	
	SoMaterial* originMaterial = new SoMaterial();
	originMaterial->diffuseColor.set1Value(0, SbColor(1, 0, 0));
	originMaterial->diffuseColor.set1Value(1, SbColor(0, 1, 0));
	originMaterial->diffuseColor.set1Value(2, SbColor(0, 0, 1));
	
	SoMaterialBinding* originMaterialBinding = new SoMaterialBinding();
	originMaterialBinding->value.setValue(SoMaterialBindingElement::PER_PART);
	
	SoIndexedLineSet* originIndexedLineSet = new SoIndexedLineSet();
	originIndexedLineSet->coordIndex.set1Value(0, 0);
	originIndexedLineSet->coordIndex.set1Value(1, 1);
	originIndexedLineSet->coordIndex.set1Value(2, SO_END_LINE_INDEX);
	originIndexedLineSet->coordIndex.set1Value(3, 0);
	originIndexedLineSet->coordIndex.set1Value(4, 2);
	originIndexedLineSet->coordIndex.set1Value(5, SO_END_LINE_INDEX);
	originIndexedLineSet->coordIndex.set1Value(6, 0);
	originIndexedLineSet->coordIndex.set1Value(7, 3);
	originIndexedLineSet->coordIndex.set1Value(8, SO_END_LINE_INDEX);
	originIndexedLineSet->materialIndex.set1Value(0, 0);
	originIndexedLineSet->materialIndex.set1Value(1, 1);
	originIndexedLineSet->materialIndex.set1Value(2, 2);
	
	SoSeparator* origin1Separator = new SoSeparator();
	origin1Separator->addChild(originCoordinate3);
	origin1Separator->addChild(originMaterial);
	origin1Separator->addChild(originMaterialBinding);
	origin1Separator->addChild(originIndexedLineSet);
	
	this->origin1Switch = new SoSwitch();
	this->origin1Switch->whichChild = SO_SWITCH_NONE;
	this->origin1Switch->addChild(origin1Separator);
	this->root->addChild(this->origin1Switch);
	
	SoScale* origin1000Scale = new SoScale();
	origin1000Scale->scaleFactor.setValue(1000, 1000, 1000);
	
	SoSeparator* origin1000Separator = new SoSeparator();
	origin1000Separator->addChild(origin1000Scale);
	origin1000Separator->addChild(originCoordinate3);
	origin1000Separator->addChild(originMaterial);
	origin1000Separator->addChild(originMaterialBinding);
	origin1000Separator->addChild(originIndexedLineSet);
	
	this->origin1000Switch = new SoSwitch();
	this->origin1000Switch->whichChild = SO_SWITCH_NONE;
	this->origin1000Switch->addChild(origin1000Separator);
	this->root->addChild(this->origin1000Switch);
	
	this->offscreenRenderer = new SoOffscreenRenderer(this->viewer->getViewportRegion());
	
	this->supportedFileEndings << ".iv" << ".iv.gz" << ".ivz" << ".wrl" << ".wrl.gz" << ".wrz";
#if defined(HAVE_SOSTLFILEKIT_H) && defined(HAVE_SOSTLFILEKIT_CONVERT)
	this->supportedFileEndings << ".stl";
#endif
	this->supportedFileEndings.sort();
	
	this->setCentralWidget(this->widget);
	this->setFocusProxy(this->viewer->getWidget());
	
	this->init();
	
	this->resize(800, 600);
	
	if (QCoreApplication::arguments().size() > 1)
	{
		this->filename = QCoreApplication::arguments()[1];
	}
	
	if (!this->filename.isEmpty())
	{
		this->load(this->filename);
	}
	
	this->viewer->viewAll();
}

MainWindow::~MainWindow()
{
	this->root->unref();
	
	if (nullptr != this->offscreenRenderer)
	{
		delete this->offscreenRenderer;
	}
}

void
MainWindow::copyCameraValues()
{
	SbVec3f axis;
	float angle;
	this->viewer->getCamera()->orientation.getValue(axis, angle);
	
	QApplication::clipboard()->setText(
		"position " +
			QString::number(this->viewer->getCamera()->position.getValue()[0]) + " " +
			QString::number(this->viewer->getCamera()->position.getValue()[1]) + " " +
			QString::number(this->viewer->getCamera()->position.getValue()[2]) + "\n" +
		"orientation " +
			QString::number(axis[0]) + " " +
			QString::number(axis[1]) + " " +
			QString::number(axis[2]) + " " +
			QString::number(angle) + "\n" +
		"nearDistance " + QString::number(this->viewer->getCamera()->nearDistance.getValue()) + "\n" +
		"farDistance " + QString::number(this->viewer->getCamera()->farDistance.getValue()) + "\n" +
		"aspectRatio " + QString::number(this->viewer->getCamera()->aspectRatio.getValue()) + "\n" +
		"focalDistance " + QString::number(this->viewer->getCamera()->focalDistance.getValue()) + "\n" +
		(
			SoPerspectiveCamera::getClassTypeId() == this->viewer->getCameraType() ?
			"heightAngle " + QString::number(static_cast<SoPerspectiveCamera*>(this->viewer->getCamera())->heightAngle.getValue()) :
			"height " + QString::number(static_cast<SoOrthographicCamera*>(this->viewer->getCamera())->height.getValue())
		) + "\n"
	);
}

void
MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	event->acceptProposedAction();
}

void
MainWindow::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasUrls())
	{
		if (event->mimeData()->urls().size() > 0)
		{
			QString filename = event->mimeData()->urls()[0].toLocalFile();
			this->load(filename);
			this->viewer->viewAll();
			event->acceptProposedAction();
		}
	}
}

void
MainWindow::init()
{
	this->fileMenu = this->menuBar()->addMenu("File");
	
	QAction* openAction = new QAction("Open...", this);
	openAction->setShortcut(QKeySequence::Open);
	QObject::connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
	this->fileMenu->addAction(openAction);
	
	this->fileMenu->addSeparator();
	
	QAction* reloadAction = new QAction("Reload", this);
	reloadAction->setShortcut(QKeySequence(QKeySequence::Refresh));
	QObject::connect(reloadAction, SIGNAL(triggered()), this, SLOT(reload()));
	this->addAction(reloadAction);
	this->fileMenu->addAction(reloadAction);
	
	this->fileMenu->addSeparator();
	
	QAction* saveImageWithoutAlphaAction = new QAction("Save Image w/o Alpha", this);
	saveImageWithoutAlphaAction->setShortcut(QKeySequence("Return"));
	QObject::connect(saveImageWithoutAlphaAction, SIGNAL(triggered()), this, SLOT(saveImageWithoutAlpha()));
	this->addAction(saveImageWithoutAlphaAction);
	this->fileMenu->addAction(saveImageWithoutAlphaAction);
		
	QAction* saveImageWithAlphaAction = new QAction("Save Image w/ Alpha", this);
	saveImageWithAlphaAction->setShortcut(QKeySequence("Shift+Return"));
	QObject::connect(saveImageWithAlphaAction, SIGNAL(triggered()), this, SLOT(saveImageWithAlpha()));
	this->addAction(saveImageWithAlphaAction);
	this->fileMenu->addAction(saveImageWithAlphaAction);
	
	this->fileMenu->addSeparator();
	
	QAction* saveImageOffscreenWithoutAlphaAction = new QAction("Save Image Offscreen w/o Alpha", this);
	saveImageOffscreenWithoutAlphaAction->setShortcut(QKeySequence("Backspace"));
	QObject::connect(saveImageOffscreenWithoutAlphaAction, SIGNAL(triggered()), this, SLOT(saveImageOffscreenWithoutAlpha()));
	this->addAction(saveImageOffscreenWithoutAlphaAction);
	this->fileMenu->addAction(saveImageOffscreenWithoutAlphaAction);
	
	QAction* saveImageOffscreenWithAlphaAction = new QAction("Save Image Offscreen w/ Alpha", this);
	saveImageOffscreenWithAlphaAction->setShortcut(QKeySequence("Shift+Backspace"));
	QObject::connect(saveImageOffscreenWithAlphaAction, SIGNAL(triggered()), this, SLOT(saveImageOffscreenWithAlpha()));
	this->addAction(saveImageOffscreenWithAlphaAction);
	this->fileMenu->addAction(saveImageOffscreenWithAlphaAction);
	
	this->fileMenu->addSeparator();
	
	QAction* exitAction = new QAction("Exit", this);
	QObject::connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
	this->fileMenu->addAction(exitAction);
	
	this->viewMenu = this->menuBar()->addMenu("View");
	
	QActionGroup* viewActionGroup = new QActionGroup(this);
	QObject::connect(viewActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(selectView(QAction*)));
	
	QAction* viewTopAction = new QAction("Top", this);
	viewTopAction->setData(static_cast<int>(View::top));
	viewActionGroup->addAction(viewTopAction);
	this->viewMenu->addAction(viewTopAction);
	
	QAction* viewBottomAction = new QAction("Bottom", this);
	viewBottomAction->setData(static_cast<int>(View::bottom));
	viewActionGroup->addAction(viewBottomAction);
	this->viewMenu->addAction(viewBottomAction);
	
	QAction* viewFrontAction = new QAction("Front", this);
	viewFrontAction->setData(static_cast<int>(View::front));
	viewActionGroup->addAction(viewFrontAction);
	this->viewMenu->addAction(viewFrontAction);
	
	QAction* viewBackAction = new QAction("Back", this);
	viewBackAction->setData(static_cast<int>(View::back));
	viewActionGroup->addAction(viewBackAction);
	this->viewMenu->addAction(viewBackAction);
	
	QAction* viewLeftAction = new QAction("Left", this);
	viewLeftAction->setData(static_cast<int>(View::left));
	viewActionGroup->addAction(viewLeftAction);
	this->viewMenu->addAction(viewLeftAction);
	
	QAction* viewRightAction = new QAction("Right", this);
	viewRightAction->setData(static_cast<int>(View::right));
	viewActionGroup->addAction(viewRightAction);
	this->viewMenu->addAction(viewRightAction);
	
	QAction* viewTopBackLeftAction = new QAction("Top Back Left", this);
	viewTopBackLeftAction->setData(static_cast<int>(View::topBackLeft));
	viewActionGroup->addAction(viewTopBackLeftAction);
	this->viewMenu->addAction(viewTopBackLeftAction);
	
	QAction* viewTopBackRightAction = new QAction("Top Back Right", this);
	viewTopBackRightAction->setData(static_cast<int>(View::topBackRight));
	viewActionGroup->addAction(viewTopBackRightAction);
	this->viewMenu->addAction(viewTopBackRightAction);
	
	QAction* viewTopFrontLeftAction = new QAction("Top Front Left", this);
	viewTopFrontLeftAction->setData(static_cast<int>(View::topFrontLeft));
	viewActionGroup->addAction(viewTopFrontLeftAction);
	this->viewMenu->addAction(viewTopFrontLeftAction);
	
	QAction* viewTopFrontRightAction = new QAction("Top Front Right", this);
	viewTopFrontRightAction->setData(static_cast<int>(View::topFrontRight));
	viewActionGroup->addAction(viewTopFrontRightAction);
	this->viewMenu->addAction(viewTopFrontRightAction);
	
	this->viewMenu->addSeparator();
	
	QAction* copyCameraValuesAction = new QAction("Copy Camera Values", this);
	QObject::connect(copyCameraValuesAction, SIGNAL(triggered()), this, SLOT(copyCameraValues()));
	this->viewMenu->addAction(copyCameraValuesAction);
	
	this->displayMenu = this->menuBar()->addMenu("Display");
	
	QAction* toggleAxisCrossAction = new QAction("Axis Cross", this);
	toggleAxisCrossAction->setCheckable(true);
	toggleAxisCrossAction->setShortcut(QKeySequence("Space"));
	QObject::connect(toggleAxisCrossAction, SIGNAL(triggered()), this, SLOT(toggleAxisCross()));
	this->addAction(toggleAxisCrossAction);
	this->displayMenu->addAction(toggleAxisCrossAction);
	
	this->displayMenu->addSeparator();
	
	QActionGroup* originUnitActionGroup = new QActionGroup(this);
	QObject::connect(originUnitActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(selectOrigin(QAction*)));
	
	QAction* originNoneAction = new QAction("Hide Origin", this);
	originNoneAction->setCheckable(true);
	originNoneAction->setChecked(true);
	originNoneAction->setData(static_cast<int>(Origin::none));
	originUnitActionGroup->addAction(originNoneAction);
	this->displayMenu->addAction(originNoneAction);
	
	QAction* origin1Action = new QAction("Show Origin (Scale 1)", this);
	origin1Action->setCheckable(true);
	origin1Action->setData(static_cast<int>(Origin::o1));
	originUnitActionGroup->addAction(origin1Action);
	this->displayMenu->addAction(origin1Action);
	
	QAction* origin1000Action = new QAction("Show Origin (Scale 1000)", this);
	origin1000Action->setCheckable(true);
	origin1000Action->setData(static_cast<int>(Origin::o1000));
	originUnitActionGroup->addAction(origin1000Action);
	this->displayMenu->addAction(origin1000Action);
	
	this->displayMenu->addSeparator();
	
	QAction* toggleFullScreenAction = new QAction("Full Screen", this);
	toggleFullScreenAction->setShortcut(QKeySequence("Alt+Return"));
	QObject::connect(toggleFullScreenAction, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
	this->displayMenu->addAction(toggleFullScreenAction);
	this->addAction(toggleFullScreenAction);
	
	this->displayMenu->addSeparator();
	
	QActionGroup* cameraActionGroup = new QActionGroup(this);
	cameraActionGroup->setExclusive(true);
	QObject::connect(cameraActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(selectCamera(QAction*)));
	
	QAction* cameraOrthogonalAction = new QAction("Orthogonal Camera", this);
	cameraOrthogonalAction->setCheckable(true);
	cameraOrthogonalAction->setData(static_cast<int>(Camera::orthogonal));
	cameraActionGroup->addAction(cameraOrthogonalAction);
	this->displayMenu->addAction(cameraOrthogonalAction);
	
	QAction* cameraPerspectiveAction = new QAction("Perspective Camera", this);
	cameraPerspectiveAction->setCheckable(true);
	cameraPerspectiveAction->setChecked(true);
	cameraPerspectiveAction->setData(static_cast<int>(Camera::perspective));
	cameraActionGroup->addAction(cameraPerspectiveAction);
	this->displayMenu->addAction(cameraPerspectiveAction);
	
	this->displayMenu->addSeparator();
	
	QActionGroup* backgroundActionGroup = new QActionGroup(this);
	backgroundActionGroup->setExclusive(true);
	QObject::connect(backgroundActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(selectBackground(QAction*)));
	
	QAction* backgroundBlackAction = new QAction("Black Background", this);
	backgroundBlackAction->setCheckable(true);
	backgroundBlackAction->setData(static_cast<int>(Background::black));
	backgroundActionGroup->addAction(backgroundBlackAction);
	this->displayMenu->addAction(backgroundBlackAction);
	
	QAction* backgroundGradientAction = new QAction("Gradient Background", this);
	backgroundGradientAction->setCheckable(true);
	backgroundGradientAction->setChecked(true);
	backgroundGradientAction->setData(static_cast<int>(Background::gradient));
	backgroundActionGroup->addAction(backgroundGradientAction);
	this->displayMenu->addAction(backgroundGradientAction);
	
	QAction* backgroundWhiteAction = new QAction("White Background", this);
	backgroundWhiteAction->setCheckable(true);
	backgroundWhiteAction->setData(static_cast<int>(Background::white));
	backgroundActionGroup->addAction(backgroundWhiteAction);
	this->displayMenu->addAction(backgroundWhiteAction);
	
	this->displayMenu->addSeparator();
	
	QActionGroup* sizeActionGroup = new QActionGroup(this);
	sizeActionGroup->setExclusive(true);
	QObject::connect(sizeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(selectSize(QAction*)));
	
	QAction* size640x480Action = new QAction("640x480", this);
	size640x480Action->setData(static_cast<int>(Size::s640x480));
	sizeActionGroup->addAction(size640x480Action);
	this->displayMenu->addAction(size640x480Action);
	
	QAction* size800x600Action = new QAction("800x600", this);
	size800x600Action->setData(static_cast<int>(Size::s800x600));
	sizeActionGroup->addAction(size800x600Action);
	this->displayMenu->addAction(size800x600Action);
	
	QAction* size1024x768Action = new QAction("1024x768", this);
	size1024x768Action->setData(static_cast<int>(Size::s1024x768));
	sizeActionGroup->addAction(size1024x768Action);
	this->displayMenu->addAction(size1024x768Action);
	
	QAction* size1024x1024Action = new QAction("1024x1024", this);
	size1024x1024Action->setData(static_cast<int>(Size::s1024x1024));
	sizeActionGroup->addAction(size1024x1024Action);
	this->displayMenu->addAction(size1024x1024Action);
	
	QAction* size1280x720Action = new QAction("1280x720", this);
	size1280x720Action->setData(static_cast<int>(Size::s1280x720));
	sizeActionGroup->addAction(size1280x720Action);
	this->displayMenu->addAction(size1280x720Action);
	
	QAction* size1280x960Action = new QAction("1280x960", this);
	size1280x960Action->setData(static_cast<int>(Size::s1280x960));
	sizeActionGroup->addAction(size1280x960Action);
	this->displayMenu->addAction(size1280x960Action);
	
	QAction* size1200x1200Action = new QAction("1200x1200", this);
	size1200x1200Action->setData(static_cast<int>(Size::s1200x1200));
	sizeActionGroup->addAction(size1200x1200Action);
	this->displayMenu->addAction(size1200x1200Action);
	
	QAction* size1600x1200Action = new QAction("1600x1200", this);
	size1600x1200Action->setData(static_cast<int>(Size::s1600x1200));
	sizeActionGroup->addAction(size1600x1200Action);
	this->displayMenu->addAction(size1600x1200Action);
	
	QAction* size1920x1080Action = new QAction("1920x1080", this);
	size1920x1080Action->setData(static_cast<int>(Size::s1920x1080));
	sizeActionGroup->addAction(size1920x1080Action);
	this->displayMenu->addAction(size1920x1080Action);
	
	QAction* size2400x2400Action = new QAction("2400x2400", this);
	size2400x2400Action->setData(static_cast<int>(Size::s2400x2400));
	sizeActionGroup->addAction(size2400x2400Action);
	this->displayMenu->addAction(size2400x2400Action);
}

void
MainWindow::inlineFetchUrlCallback(const SbString& url, void* userData, SoVRMLInline* node)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(userData);
	
	QString string(node->url.getValues(0)->getString());
	
	if (string.startsWith("http://") || string.startsWith("ftp://"))
	{
		QNetworkReply* reply = mainWindow->manager->get(QNetworkRequest(QUrl(node->url.getValues(0)->getString())));
		reply->setProperty("node", QVariant::fromValue(static_cast<void*>(node)));
	}
	else
	{
		if (!mainWindow->input.pushFile(node->url.getValues(0)->getString()))
		{
			return;
		}
		
		SoSeparator* root = SoDB::readAll(&mainWindow->input);
		
		if (nullptr == root)
		{
			if (mainWindow->input.getCurFileName() == node->getFullURLName())
			{
				char dummy;
				while (!mainWindow->input.eof() && mainWindow->input.get(dummy));
				assert(mainWindow->input.eof());
				
				if (mainWindow->input.get(dummy))
				{
					mainWindow->input.putBack(dummy);
				}
			}
			
			SoReadError::post(&mainWindow->input, "Unable to read Inline file: ``%s''", node->url.getValues(0)->getString());
			return;
		}
		
		root->ref();
		node->setChildData(root);
		root->unref();
	}
}

void
MainWindow::load(const QString filename)
{
	QFileInfo fileInfo(filename);
	QString absoluteFilename = QDir::toNativeSeparators(QDir::cleanPath(fileInfo.absoluteFilePath()));
	QDir::setCurrent(fileInfo.path());
	
	bool fileEndingSupported = false;
	
	for (int i = 0; i < this->supportedFileEndings.size(); ++i)
	{
		fileEndingSupported |= absoluteFilename.endsWith(this->supportedFileEndings[i], Qt::CaseInsensitive);
	}
	
	if (!fileEndingSupported)
	{
		QMessageBox::critical(this, "Error", "Format of file '" + absoluteFilename + "' not supported.");
		return;
	}
	
	if (nullptr != this->scene)
	{
		this->root->removeChild(this->scene);
		this->filename.clear();
		this->setWindowTitle("wrlview");
	}
	
#if defined(HAVE_SOSTLFILEKIT_H) && defined(HAVE_SOSTLFILEKIT_CONVERT)
	if (absoluteFilename.endsWith(".stl", Qt::CaseInsensitive))
	{
		SoSTLFileKit* stlFileKit = new ::SoSTLFileKit();
		stlFileKit->ref();
		
		if (!stlFileKit->readFile(absoluteFilename.toStdString().c_str()))
		{
			stlFileKit->unref();
			QMessageBox::critical(this, "Error", "Error reading file '" + absoluteFilename + "'.");
			return;
		}
		
		this->scene = stlFileKit->convert();
		stlFileKit->unref();
	}
	else
#endif
	{
		if (!this->input.openFile(absoluteFilename.toStdString().c_str(), true))
		{
			QMessageBox::critical(this, "Error", "File '" + absoluteFilename + "' not found.");
			return;
		}
		
		this->scene = SoDB::readAll(&this->input);
		
		this->input.closeFile();
	}
	
	if (nullptr == this->scene)
	{
		QMessageBox::critical(this, "Error", "Error reading file '" + absoluteFilename + "'.");
		return;
	}
	
	this->root->addChild(this->scene);
	this->filename = absoluteFilename;
	this->setWindowTitle(absoluteFilename + " - wrlview");
}

void
MainWindow::open()
{
	QString filename = QFileDialog::getOpenFileName(this, "", this->filename, "All Formats (*" + this->supportedFileEndings.join(" *") + ")");
	
	if (!filename.isEmpty())
	{
		this->load(filename);
	}
	
	this->viewer->viewAll();
}

void
MainWindow::reload()
{
	if (!this->filename.isEmpty())
	{
		this->load(this->filename);
	}
}

void
MainWindow::replyFinished(QNetworkReply* reply)
{
	QByteArray data = reply->readAll();
	
	SoInput input;
	input.setBuffer(data.data(), data.size());
	
	SoSeparator* root = SoDB::readAll(&input);
	
	if (nullptr == root)
	{
		SoReadError::post(&this->input, "Unable to read Inline file: ``%s''", reply->url().toString().toStdString().c_str());
		return;
	}
	
	root->ref();
	SoVRMLInline* node = static_cast<SoVRMLInline*>(reply->property("node").value<void*>());
	node->setChildData(root);
	root->unref();
	
	this->viewer->viewAll();
}

void
MainWindow::saveImage(bool withAlpha)
{
	QString filename = "wrlview-" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmsszzz") + ".png";
	
	glReadBuffer(GL_FRONT);
	
#if QT_VERSION >= 0x060000
	QOpenGLWindow* window = this->viewer->getGLWidget()->property("SoQtGLArea").value<QOpenGLWindow*>();
	QSurfaceFormat surfaceFormat = window->format();
	
	if (withAlpha != surfaceFormat.hasAlpha())
	{
		surfaceFormat.setAlphaBufferSize(withAlpha ? 8 : 0);
		window->setFormat(surfaceFormat);
	}
	
	QImage image = window->grabFramebuffer();
#else
	QImage image = static_cast<QGLWidget*>(this->viewer->getGLWidget())->grabFrameBuffer(withAlpha);
#endif
	
	QString format = filename.right(filename.length() - filename.lastIndexOf('.') - 1).toUpper();
	
	if (("JFIF" == format) || ("JPE" == format) || ("JPG" == format))
	{
		format = "JPEG";
	}

	if (!image.save(filename, format.toStdString().c_str()))
	{
		QMessageBox::critical(this, "Error", "Error writing " + filename + ".");
	}
}

void
MainWindow::saveImageOffscreen(bool withAlpha)
{
	QString filename = "wrlview-" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmsszzz") + ".png";
	
	SbViewportRegion viewportRegion(this->viewer->getViewportRegion());
	viewportRegion.setPixelsPerInch(300);
	viewportRegion.setWindowSize(
		viewportRegion.getViewportSizePixels()[0] * viewportRegion.getPixelsPerInch() / SoOffscreenRenderer::getScreenPixelsPerInch(),
		viewportRegion.getViewportSizePixels()[1] * viewportRegion.getPixelsPerInch() / SoOffscreenRenderer::getScreenPixelsPerInch()
	);
	
	this->offscreenRenderer->setBackgroundColor(this->viewer->getBackgroundColor());
	this->offscreenRenderer->setComponents(withAlpha ? SoOffscreenRenderer::RGB_TRANSPARENCY : SoOffscreenRenderer::RGB);
	this->offscreenRenderer->setViewportRegion(viewportRegion);
	this->offscreenRenderer->getGLRenderAction()->setNumPasses(8);
	
	if (!this->offscreenRenderer->render(this->viewer->getSceneManager()->getSceneGraph()))
	{
		QMessageBox::critical(this, "Error", "Error rendering " + filename + ".");
		return;
	}
	
	if (!this->offscreenRenderer->isWriteSupported("png"))
	{
		QMessageBox::critical(this, "Error", "Filetype PNG not supported.");
		return;
	}
	
	if (!this->offscreenRenderer->writeToFile(filename.toStdString().c_str(), "png"))
	{
		QMessageBox::critical(this, "Error", "Error writing " + filename + ".");
		return;
	}
}

void
MainWindow::saveImageOffscreenWithAlpha()
{
	this->saveImageOffscreen(true);
}

void
MainWindow::saveImageOffscreenWithoutAlpha()
{
	this->saveImageOffscreen(false);
}

void
MainWindow::saveImageWithAlpha()
{
	this->saveImage(true);
}

void
MainWindow::saveImageWithoutAlpha()
{
	this->saveImage(false);
}

void
MainWindow::selectBackground(QAction* action)
{
	switch (static_cast<Background>(action->data().toInt()))
	{
	case Background::black:
		this->backgroundSwitch->whichChild = SO_SWITCH_NONE;
		this->viewer->setBackgroundColor(SbColor(0, 0, 0));
		break;
	case Background::gradient:
		this->backgroundSwitch->whichChild = SO_SWITCH_ALL;
		break;
	case Background::white:
		this->backgroundSwitch->whichChild = SO_SWITCH_NONE;
		this->viewer->setBackgroundColor(SbColor(255, 255, 255));
		break;
	default:
		break;
	}
}

void
MainWindow::selectCamera(QAction* action)
{
	switch (static_cast<Camera>(action->data().toInt()))
	{
	case Camera::orthogonal:
		this->viewer->setCameraType(SoOrthographicCamera::getClassTypeId());
		break;
	case Camera::perspective:
		this->viewer->setCameraType(SoPerspectiveCamera::getClassTypeId());
		break;
	default:
		break;
	}
	
	this->viewer->getCamera()->setToDefaults();
	this->viewer->viewAll();
}

void
MainWindow::selectOrigin(QAction* action)
{
	switch (static_cast<Origin>(action->data().toInt()))
	{
	case Origin::none:
		this->origin1Switch->whichChild = SO_SWITCH_NONE;
		this->origin1000Switch->whichChild = SO_SWITCH_NONE;
		break;
	case Origin::o1:
		this->origin1000Switch->whichChild = SO_SWITCH_NONE;
		this->origin1Switch->whichChild = SO_SWITCH_ALL;
		break;
	case Origin::o1000:
		this->origin1Switch->whichChild = SO_SWITCH_NONE;
		this->origin1000Switch->whichChild = SO_SWITCH_ALL;
		break;
	default:
		break;
	}
}

void
MainWindow::selectSize(QAction* action)
{
	switch (static_cast<Size>(action->data().toInt()))
	{
	case Size::s640x480:
		this->widget->setFixedSize(640, 480);
		break;
	case Size::s800x600:
		this->widget->setFixedSize(800, 600);
		break;
	case Size::s1024x768:
		this->widget->setFixedSize(1024, 768);
		break;
	case Size::s1024x1024:
		this->widget->setFixedSize(1024, 1024);
		break;
	case Size::s1280x720:
		this->widget->setFixedSize(1280, 720);
		break;
	case Size::s1280x960:
		this->widget->setFixedSize(1280, 960);
		break;
	case Size::s1200x1200:
		this->widget->setFixedSize(1200, 1200);
		break;
	case Size::s1600x1200:
		this->widget->setFixedSize(1600, 1200);
		break;
	case Size::s1920x1080:
		this->widget->setFixedSize(1920, 1080);
		break;
	case Size::s2400x2400:
		this->widget->setFixedSize(2400, 2400);
		break;
	default:
		break;
	}
	
	this->setFixedSize(this->widget->sizeHint().width(), this->widget->sizeHint().height());
	this->widget->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
	this->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

void
MainWindow::selectView(QAction* action)
{
	this->viewer->getCamera()->setToDefaults();
	
	SbVec3f position;
	SbVec3f upvector(0, 0, 1);
	
	switch (static_cast<View>(action->data().toInt()))
	{
	case View::back:
		position.setValue(0, -1, 0);
		break;
	case View::bottom:
		position.setValue(0, 0, -1);
		upvector.setValue(0, -1, 0);
		break;
	case View::front:
		position.setValue(0, 1, 0);
		break;
	case View::left:
		position.setValue(-1, 0, 0);
		break;
	case View::right:
		position.setValue(1, 0, 0);
		break;
	case View::top:
		position.setValue(0, 0, 1);
		upvector.setValue(0, 1, 0);
		break;
	case View::topBackLeft:
		position.setValue(-1, -1, 1);
		break;
	case View::topBackRight:
		position.setValue(1, -1, 1);
		break;
	case View::topFrontLeft:
		position.setValue(-1, 1, 1);
		break;
	case View::topFrontRight:
		position.setValue(1, 1, 1);
		break;
	default:
		break;
	}
	
	this->viewer->getCamera()->position.setValue(position);
	this->viewer->getCamera()->pointAt(SbVec3f(0, 0, 0), upvector);
	this->viewer->getCamera()->scaleHeight(1);
	this->viewer->viewAll();
}

void
MainWindow::toggleAxisCross()
{
	if (this->viewer->isFeedbackVisible())
	{
		this->viewer->setFeedbackVisibility(false);
	}
	else
	{
		this->viewer->setFeedbackVisibility(true);
	}
}

void
MainWindow::toggleFullScreen()
{
	if (this->isFullScreen())
	{
		this->showNormal();
		this->menuBar()->show();
	}
	else
	{
		this->showFullScreen();
		this->menuBar()->hide();
	}
}
