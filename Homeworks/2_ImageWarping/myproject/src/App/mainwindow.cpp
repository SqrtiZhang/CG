#include "mainwindow.h"
#include <QtWidgets>
#include <QImage>
#include <QPainter>
#include "ImageWidget.h"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	//ui.setupUi(this);

	setGeometry(300, 150, 800, 450);

	imagewidget_ = new ImageWidget();
	setCentralWidget(imagewidget_);

	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();
}

MainWindow::~MainWindow()
{
	delete imagewidget_;
}

void MainWindow::closeEvent(QCloseEvent *e)
{

}

void MainWindow::paintEvent(QPaintEvent* paintevent)
{
	
}

void MainWindow::CreateActions()
{
    action_new_ = new QAction(QIcon(":/MainWindow/Resources/images/new.png"), tr("&New"), this);
	action_new_->setShortcut(QKeySequence::New);
	action_new_->setStatusTip(tr("Create a new file"));
	// connect ...

    action_open_ = new QAction(QIcon(":/MainWindow/Resources/images/open.png"), tr("&Open..."), this);
	action_open_->setShortcuts(QKeySequence::Open);
	action_open_->setStatusTip(tr("Open an existing file"));
	connect(action_open_, &QAction::triggered, imagewidget_, &ImageWidget::Open);

    action_save_ = new QAction(QIcon(":/MainWindow/Resources/images/save.png"), tr("&Save"), this);
	action_save_->setShortcuts(QKeySequence::Save);
	action_save_->setStatusTip(tr("Save the document to disk"));
    connect(action_save_,&QAction::triggered, imagewidget_, &ImageWidget::Save);

    action_saveas_ = new QAction(QIcon(":/MainWindow/Resources/images/saveas.png"),tr("Save &As..."), this);
	action_saveas_->setShortcuts(QKeySequence::SaveAs);
	action_saveas_->setStatusTip(tr("Save the document under a new name"));
	connect(action_saveas_, &QAction::triggered, imagewidget_, &ImageWidget::SaveAs);

    action_invert_ = new QAction(QIcon(":/MainWindow/Resources/images/inverse.png"),tr("Inverse"), this);
    action_invert_->setStatusTip(tr("Invert all pixel value in the image"));
	connect(action_invert_, &QAction::triggered, imagewidget_, &ImageWidget::Invert);

    action_mirror_ = new QAction(QIcon(":/MainWindow/Resources/images/mirror.png"),tr("Mirror"), this);
	action_mirror_->setStatusTip(tr("Mirror image vertically or horizontally"));
	// The slot requires more arguments than the signal provides.
	//connect(action_mirror_, &QAction::triggered, imagewidget_, &ImageWidget::Mirror);
	connect(action_mirror_, &QAction::triggered, [this]() {
		imagewidget_->Mirror(); // use default arguments
		});

    action_gray_ = new QAction(QIcon(":/MainWindow/Resources/images/gray.png"),tr("Grayscale"), this);
	action_gray_->setStatusTip(tr("Gray-scale map"));
	connect(action_gray_, &QAction::triggered, imagewidget_, &ImageWidget::TurnGray);

    action_restore_ = new QAction(QIcon(":/MainWindow/Resources/images/restore.png"),tr("Restore"), this);
	action_restore_->setStatusTip(tr("Show origin image"));
	connect(action_restore_, &QAction::triggered, imagewidget_, &ImageWidget::Restore);

    action_input_=new QAction(QIcon(":/MainWindow/Resources/images/input.png"),tr("Input"),this);
    action_input_->setStatusTip(tr("Input points"));
    connect(action_input_, &QAction::triggered, imagewidget_, &ImageWidget::Input);

    action_idw_=new QAction(tr("IDW"),this);
    action_idw_->setStatusTip(tr("Warp by IDW"));
    connect(action_idw_, &QAction::triggered, imagewidget_, &ImageWidget::DoIDW);

    action_rbf_=new QAction(tr("RBF"),this);
    action_idw_->setStatusTip(tr("Warp by RBF"));
    connect(action_rbf_, &QAction::triggered, imagewidget_, &ImageWidget::DoRBF);

}

void MainWindow::CreateMenus()
{
	menu_file_ = menuBar()->addMenu(tr("&File"));
	menu_file_->setStatusTip(tr("File menu"));
	menu_file_->addAction(action_new_);
	menu_file_->addAction(action_open_);
	menu_file_->addAction(action_save_);
	menu_file_->addAction(action_saveas_);

	menu_edit_ = menuBar()->addMenu(tr("&Edit"));
	menu_edit_->setStatusTip(tr("Edit menu"));
	menu_edit_->addAction(action_invert_);
	menu_edit_->addAction(action_mirror_);
	menu_edit_->addAction(action_gray_);
    menu_edit_->addAction(action_input_);
	menu_edit_->addAction(action_restore_);
    menu_edit_->addAction(action_idw_);
    menu_edit_->addAction(action_rbf_);
}

void MainWindow::CreateToolBars()
{
	toolbar_file_ = addToolBar(tr("File"));
	toolbar_file_->addAction(action_new_);
	toolbar_file_->addAction(action_open_);
	toolbar_file_->addAction(action_save_);

	// Add separator in toolbar 
	toolbar_file_->addSeparator();
	toolbar_file_->addAction(action_invert_);
	toolbar_file_->addAction(action_mirror_);
	toolbar_file_->addAction(action_gray_);
	toolbar_file_->addAction(action_restore_);
    toolbar_file_->addAction(action_idw_);
    toolbar_file_->addAction(action_rbf_);
    toolbar_file_->addAction(action_input_);

}

void MainWindow::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}
