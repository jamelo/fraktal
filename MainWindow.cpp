#include "MainWindow.h"

#include <thread>
#include <complex>

#include <KApplication>
#include <KAction>
#include <KStandardAction>
#include <KActionCollection>
#include <KMenu>
#include <KStatusBar>
#include <KToolBar>
#include <KLocale>

#include <QSignalMapper>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QPixmap>

#include "Wrapper.h"
#include "RenderParams.h"
#include "Canvas.h"

MainWindow::MainWindow(QWidget* parent) :
    KXmlGuiWindow(parent),
    m_antialiasingAmount(1),
    m_colorScheme(ColorScheme::Grey),
    m_zoomRegion(ZoomRegion(-2, -1, 1, 1)),
    m_canvas(nullptr),
    m_progressBar(nullptr)
{
    this->setupWidgets();
    this->setupActions();
    this->setupGUI(Default, "fractal-viewerui.rc");

    this->stateChanged("idle");

    connect(m_canvas->backgroundWorker(), SIGNAL(taskStart()), this, SLOT(previewStart()));
    connect(m_canvas->backgroundWorker(), SIGNAL(taskComplete(bool)), this, SLOT(previewComplete(bool)));
    connect(m_canvas->backgroundWorker(), SIGNAL(progressUpdate(int)), m_progressBar, SLOT(setValue(int)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupWidgets()
{
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(50);
    m_progressBar->setMinimumWidth(150);
    m_progressBar->setMaximumWidth(150);
    m_progressBar->setVisible(false);
    this->statusBar()->addPermanentWidget(m_progressBar, 0);

    this->statusBar()->showMessage("Test");
    this->statusBar()->setSizeGripEnabled(true);

    m_canvas = new Canvas(this);
    this->setCentralWidget(m_canvas);
}

void MainWindow::setupActions()
{
    KAction* actionQuit = KStandardAction::quit(kapp, SLOT(quit()), this->actionCollection());
    KAction* actionSaveAs = KStandardAction::saveAs(this, SLOT(saveAs()), this->actionCollection());
    actionSaveAs->setStatusTip("Saves parameters to a file for later viewing.");

    KAction* actionRender = new KAction(this);
    actionRender->setText(i18n("&Render"));
    actionRender->setIcon(KIcon ("document-save"));
    actionRender->setShortcut(Qt::CTRL + Qt::Key_R);
    actionRender->setStatusTip("Renders the fractal with the current parameters to an image file.");
    this->actionCollection()->addAction("actionRender", actionRender);

    KAction* actionStop = new KAction(this);
    actionStop->setText(i18n("&Stop"));
    actionStop->setIcon(KIcon ("process-stop"));
    actionStop->setShortcut(Qt::Key_Escape);
    actionStop->setStatusTip("Cancels rendering.");
    this->actionCollection()->addAction("actionStop", actionStop);
    connect(actionStop, SIGNAL(triggered(bool)), this, SLOT(stop()));

    KAction* actionZoomIn = new KAction(this);
    actionZoomIn->setText(i18n("Zoom &In"));
    actionZoomIn->setIcon(KIcon ("zoom-in"));
    actionZoomIn->setShortcut(Qt::CTRL + Qt::Key_Plus);
    this->actionCollection()->addAction("actionZoomIn", actionZoomIn);

    KAction* actionZoomOut = new KAction(this);
    actionZoomOut->setText(i18n("Zoom &Out"));
    actionZoomOut->setIcon(KIcon ("zoom-out"));
    actionZoomOut->setShortcut(Qt::CTRL + Qt::Key_Minus);
    this->actionCollection()->addAction("actionZoomOut", actionZoomOut);

    KAction* actionZoomReset = new KAction(this);
    actionZoomReset->setText(i18n("&Reset Zoom"));
    actionZoomReset->setIcon(KIcon ("zoom-original"));
    actionZoomReset->setShortcut(Qt::CTRL + Qt::Key_0);
    this->actionCollection()->addAction("actionZoomReset", actionZoomReset);
    this->connect(actionZoomReset, SIGNAL(triggered(bool)), this, SLOT(zoomReset()));

    QSignalMapper* colorMapper = new QSignalMapper(this);

    KAction* actionColorFire = new KAction(this);
    actionColorFire->setText(i18n("&Fire"));
    actionColorFire->setCheckable(true);
    this->actionCollection()->addAction("actionColorFire", actionColorFire);
    this->connect(actionColorFire, SIGNAL(triggered(bool)), colorMapper, SLOT(map()));

    KAction* actionColorIce = new KAction(this);
    actionColorIce->setText(i18n("&Ice"));
    actionColorIce->setCheckable(true);
    this->actionCollection()->addAction("actionColorIce", actionColorIce);
    this->connect(actionColorIce, SIGNAL(triggered(bool)), colorMapper, SLOT(map()));

    KAction* actionColorRainbow = new KAction(this);
    actionColorRainbow->setText(i18n("&Rainbow"));
    actionColorRainbow->setCheckable(true);
    this->actionCollection()->addAction("actionColorRainbow", actionColorRainbow);
    this->connect(actionColorRainbow, SIGNAL(triggered(bool)), colorMapper, SLOT(map()));

    KAction* actionColorYellowBlue = new KAction(this);
    actionColorYellowBlue->setText(i18n("&Yellow and Blue"));
    actionColorYellowBlue->setCheckable(true);
    this->actionCollection()->addAction("actionColorYellowBlue", actionColorYellowBlue);
    this->connect(actionColorYellowBlue, SIGNAL(triggered(bool)), colorMapper, SLOT(map()));

    KAction* actionColorGreenYellow = new KAction(this);
    actionColorGreenYellow->setText(i18n("&Green and Yellow"));
    actionColorGreenYellow->setCheckable(true);
    this->actionCollection()->addAction("actionColorGreenYellow", actionColorGreenYellow);
    this->connect(actionColorGreenYellow, SIGNAL(triggered(bool)), colorMapper, SLOT(map()));

    KAction* actionColorCustom = new KAction(this);
    actionColorCustom->setText(i18n("&Custom"));
    actionColorCustom->setCheckable(true);
    this->actionCollection()->addAction("actionColorCustom", actionColorCustom);
    this->connect(actionColorCustom, SIGNAL(triggered(bool)), this, SLOT(customColorScheme()));

    colorMapper->setMapping(actionColorFire, new Wrapper<ColorScheme>(this, ColorScheme::Fire));
    colorMapper->setMapping(actionColorIce, new Wrapper<ColorScheme>(this, ColorScheme::Ice));
    colorMapper->setMapping(actionColorRainbow, new Wrapper<ColorScheme>(this, ColorScheme::Rainbow));
    colorMapper->setMapping(actionColorYellowBlue, new Wrapper<ColorScheme>(this, ColorScheme::YellowBlue));
    colorMapper->setMapping(actionColorGreenYellow, new Wrapper<ColorScheme>(this, ColorScheme::GreenYellow));

    connect(colorMapper, SIGNAL(mapped(QObject*)), this, SLOT(changeColorScheme(QObject*)));

    QSignalMapper* aaMapper = new QSignalMapper(this);

    KAction* actionAANone = new KAction(this);
    actionAANone->setText(i18n("&None"));
    actionAANone->setCheckable(true);
    this->actionCollection()->addAction("actionAANone", actionAANone);
    this->connect(actionAANone, SIGNAL(triggered(bool)), aaMapper, SLOT(map()));

    KAction* actionAA2x = new KAction(this);
    actionAA2x->setText(i18n("&2x"));
    actionAA2x->setCheckable(true);
    this->actionCollection()->addAction("actionAA2x", actionAA2x);
    this->connect(actionAA2x, SIGNAL(triggered(bool)), aaMapper, SLOT(map()));

    KAction* actionAA4x = new KAction(this);
    actionAA4x->setText(i18n("&4x"));
    actionAA4x->setCheckable(true);
    this->actionCollection()->addAction("actionAA4x", actionAA4x);
    this->connect(actionAA4x, SIGNAL(triggered(bool)), aaMapper, SLOT(map()));

    KAction* actionAA6x = new KAction(this);
    actionAA6x->setText(i18n("&6x"));
    actionAA6x->setCheckable(true);
    this->actionCollection()->addAction("actionAA6x", actionAA6x);
    this->connect(actionAA6x, SIGNAL(triggered(bool)), aaMapper, SLOT(map()));

    KAction* actionAA8x = new KAction(this);
    actionAA8x->setText(i18n("&8x"));
    actionAA8x->setCheckable(true);
    this->actionCollection()->addAction("actionAA8x", actionAA8x);
    this->connect(actionAA8x, SIGNAL(triggered(bool)), aaMapper, SLOT(map()));

    KAction* actionAA16x = new KAction(this);
    actionAA16x->setText(i18n("&16x"));
    actionAA16x->setCheckable(true);
    this->actionCollection()->addAction("actionAA16x", actionAA16x);
    this->connect(actionAA16x, SIGNAL(triggered(bool)), aaMapper, SLOT(map()));

    KAction* actionAA32x = new KAction(this);
    actionAA32x->setText(i18n("&32x"));
    actionAA32x->setCheckable(true);
    this->actionCollection()->addAction("actionAA32x", actionAA32x);
    this->connect(actionAA32x, SIGNAL(triggered(bool)), aaMapper, SLOT(map()));

    aaMapper->setMapping(actionAANone, 1);
    aaMapper->setMapping(actionAA2x, 2);
    aaMapper->setMapping(actionAA4x, 4);
    aaMapper->setMapping(actionAA6x, 6);
    aaMapper->setMapping(actionAA8x, 8);
    aaMapper->setMapping(actionAA16x, 16);
    aaMapper->setMapping(actionAA32x, 32);

    connect(aaMapper, SIGNAL(mapped(int)), this, SLOT(changeAntiAliasing(int)));

    QActionGroup* colorGroup = new QActionGroup(this);
    colorGroup->addAction(actionColorFire);
    colorGroup->addAction(actionColorIce);
    colorGroup->addAction(actionColorRainbow);
    colorGroup->addAction(actionColorGreenYellow);
    colorGroup->addAction(actionColorYellowBlue);
    colorGroup->addAction(actionColorCustom);
    actionColorFire->setChecked(true);

    QActionGroup* aaGroup = new QActionGroup(this);
    aaGroup->addAction(actionAANone);
    aaGroup->addAction(actionAA2x);
    aaGroup->addAction(actionAA4x);
    aaGroup->addAction(actionAA6x);
    aaGroup->addAction(actionAA8x);
    aaGroup->addAction(actionAA16x);
    aaGroup->addAction(actionAA32x);
    actionAANone->setChecked(true);

    KMenu* colorMenu = new KMenu("Colors", this);
    colorMenu->addAction(actionColorFire);
    colorMenu->addAction(actionColorIce);
    colorMenu->addAction(actionColorRainbow);
    colorMenu->addAction(actionColorGreenYellow);
    colorMenu->addAction(actionColorYellowBlue);
    colorMenu->addAction(actionColorCustom);

    KAction* actionColors = new KAction(this);
    actionColors->setText("&Colors");
    actionColors->setIcon(KIcon ("fill-color"));
    actionColors->setMenu(colorMenu);
    actionColors->setStatusTip("Select a color scheme for the fractal.");
    this->actionCollection()->addAction("actionColors", actionColors);

    KMenu* antialiasingMenu = new KMenu("Antialiasing");
    antialiasingMenu->addAction(actionAANone);
    antialiasingMenu->addAction(actionAA2x);
    antialiasingMenu->addAction(actionAA4x);
    antialiasingMenu->addAction(actionAA6x);
    antialiasingMenu->addAction(actionAA8x);
    antialiasingMenu->addAction(actionAA16x);
    antialiasingMenu->addAction(actionAA32x);

    KAction* actionAntialiasing = new KAction(this);
    actionAntialiasing->setText("&Antialiasing");
    actionAntialiasing->setMenu(antialiasingMenu);
    actionAntialiasing->setStatusTip("Select the rendering quality.");
    this->actionCollection()->addAction("actionAntialiasing", actionAntialiasing);
}

void MainWindow::render()
{

}

void MainWindow::stop()
{
    //TODO: wire up stop action
}

void MainWindow::previewStart()
{
    //TODO: only execute previewStart on high quality preview (not sketch)
    m_progressBar->setVisible(true);
    this->stateChanged("calculatingPreview");
}

void MainWindow::previewComplete(bool canceled)
{
    m_progressBar->setVisible(false);
    this->stateChanged("idle");
}

void MainWindow::customColorScheme()
{
    //TODO: create color scheme dialog
}

void MainWindow::saveAs()
{

}

void MainWindow::zoomIn()
{
    //TODO: wire up zoom in button
}

void MainWindow::zoomOut()
{
    //TODO: wire up zoom out button
}

void MainWindow::zoomReset()
{
    //TODO: wire up zoom reset button
}

void MainWindow::changeAntiAliasing ( int amount )
{
    //TODO: wire up antialiasing menu
}

void MainWindow::changeColorScheme ( QObject* colors )
{
    Wrapper<ColorScheme>* colorSchemeWrapper = dynamic_cast<Wrapper<ColorScheme>*>(colors);

    //TODO: wire up color scheme menu
}

