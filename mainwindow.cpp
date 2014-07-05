#include "mainwindow.h"
#include "ui_mainwindow.h"




//need to be installed :
//- sudo apt-get install libgtk2.0-dev libappindicator-dev libnotify-dev




//TODO
//- put icon for the downloaded and not yet downloaded
//- ajouter un status en cours de dl dans le tableau
//* ajouter l'option de minimisation de l'application : https://qt-project.org/doc/qt-4.8/desktop-systray.html



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  this->downloadEnable = true;

  settings = new QSettings(QString("configs/config.ini"), QSettings::IniFormat);
  ui->downloadDestination->setText(settings->value("destination", "").toString());

  modelListVideo = new QStandardItemModel(0, 0, this);

  this->currentlyDownloading = false;
  this->YoutubeDlInstalled = false;
  installYoutubeDl();

  rssFeed = new RssFeed("https://gdata.youtube.com/feeds/api/users/fczJ-auI5DysJ2n-cvh0Sg/newsubscriptionvideos", settings);

  connect(rssFeed, SIGNAL(doneReading()), this, SLOT(displayingVideos()));


  createTrayIcon();
  //trayIcon->show();


  timer = new QTimer();
  //timer->setInterval(10*60*1000);
  timer->setInterval(10*1000);
  timer->start();

  connect(timer, SIGNAL(timeout()), this, SLOT(recheckFeed()));
}

MainWindow::~MainWindow()
{
  delete installProc;
  installProc = new QProcess();
  installProc->start("/bin/bash", QStringList() << "-c" << "rm -r youtube-dl");


  settings->setValue("destination", "/home/karlito/Downloads/a_voir/");
  settings->sync();

  delete ui;
}


void MainWindow::displayingVideos(){

  listVideos = rssFeed->getListVideos();

  modelListVideo->clear();
  modelListVideo->setColumnCount(3);
  modelListVideo->setRowCount(listVideos->count());

  modelListVideo->setHorizontalHeaderItem(0, new QStandardItem(QString("Title")));
  modelListVideo->setHorizontalHeaderItem(1, new QStandardItem(QString("Code")));
  modelListVideo->setHorizontalHeaderItem(2, new QStandardItem(QString("Downloaded")));

  Video *vid;
  for(int i=0; i<listVideos->count(); i++){

    vid = listVideos->at(i);

    modelListVideo->setItem(i, 0, new QStandardItem(vid->getTitle()));
    modelListVideo->setItem(i, 1, new QStandardItem(vid->getCode()));
    if(vid->haveAlreadyBeenDownloaded()) modelListVideo->setItem(i, 2, new QStandardItem("yes"));
    else if(vid->isCurrentlyDownloading()) modelListVideo->setItem(i, 2, new QStandardItem("downloading"));
    else modelListVideo->setItem(i, 2, new QStandardItem("no"));
  }

  ui->widgetListVideos->setModel(modelListVideo);

  if(!currentlyDownloading) downloadVideo();
}



void MainWindow::downloadVideo(){

  if( (this->downloadEnable) && (this->YoutubeDlInstalled) ){

    QList<Video *> *listvid = rssFeed->getListVideos();
    for(int i=0; i<listvid->count(); i++){

      if(!listvid->at(i)->haveAlreadyBeenDownloaded()){

        connect(listvid->at(i), SIGNAL(videoDownloaded(Video *)), this, SLOT(videoDoneDownloading(Video *)));
        connect(listvid->at(i), SIGNAL(videoDownloadStarted(Video*)), this, SLOT(videoStartDownloading(Video*)));
        connect(this, SIGNAL(stopDownloading()), listvid->at(i), SLOT(stopDownload()));

        listvid->at(i)->download();

        break;
      }
    }
  }
}



void MainWindow::installYoutubeDl(){

  installProc = new QProcess();
  installProc->start("/bin/bash", QStringList() << "-c" << "wget http://yt-dl.org/latest/youtube-dl.tar.gz && tar -xvf youtube-dl.tar.gz && rm youtube-dl.tar.gz");

  connect(installProc, SIGNAL(finished(int)), this, SLOT(doneInstallingYoutubeDl()));

}

void MainWindow::doneInstallingYoutubeDl(){

  this->YoutubeDlInstalled = true;
}

void MainWindow::videoStartDownloading(Video *vid){

  this->currentlyDownloading = true;
  displayingVideos();
}

void MainWindow::videoDoneDownloading(Video *vid){

  qDebug() << "done";

  disconnect(vid, SLOT(stopDownload()));
  this->currentlyDownloading = false;
  //TODO : add icon to notifications :
  // -i /usr/share/pixmaps/idle.xpm
  system("notify-send 'Video downloaded' '"+vid->getTitle().toUtf8()+"' '-t' 5000");
  displayingVideos();
}

void MainWindow::on_browse_clicked()
{
  QString path = QFileDialog::getExistingDirectory (this, tr("Directory"));
  if ( path.isNull() == false )
  {
      ui->downloadDestination->setText(path);
  }
}

void MainWindow::on_downloadDestination_textChanged()
{
  settings->setValue("destination", ui->downloadDestination->text());
}

void MainWindow::on_Download_clicked(bool checked)
{
  this->downloadEnable = checked;
  if(!this->downloadEnable) emit stopDownloading();
  else displayingVideos();
}


void MainWindow::recheckFeed(){

  rssFeed->fetch();
}


void MainWindow::createTrayIcon(){

    /*trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(new QAction(tr("Mi&nimize"), this));*/

    /*trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);*/

    /*trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/images/Wario.ico"));*/


  QString desktop;
  bool isUnity;

  desktop = getenv("XDG_CURRENT_DESKTOP");
  isUnity = (desktop.toLower() == "unity");

  if(isUnity) //only use this in unity
  {
      AppIndicator *indicator;
      GtkWidget *menu;
      GtkWidget *showItem;
      GtkWidget *quitItem;

      menu = gtk_menu_new();

      //show Item
      showItem = gtk_check_menu_item_new_with_label("Show");
      gtk_check_menu_item_set_active((GtkCheckMenuItem*)showItem, true);
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), showItem);
      g_signal_connect(showItem, "toggled", G_CALLBACK(MainWindow::showWindow), qApp);

      //g_signal_connect()
      gtk_widget_show(showItem);

      //quit item
      quitItem = gtk_menu_item_new_with_label("Quit");
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), quitItem);
      g_signal_connect(quitItem, "activate", G_CALLBACK(MainWindow::quitWindow), qApp);
      gtk_widget_show(quitItem);

      indicator = app_indicator_new("example", "indicator-messages", APP_INDICATOR_CATEGORY_OTHER);

      app_indicator_set_icon_theme_path(indicator, THEME_PATH);
      app_indicator_set_icon_full(indicator, "icon", "description");

      app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
      app_indicator_set_menu(indicator, GTK_MENU(menu));
  }
  else //other DE's and OS's
  {

  }

}




void MainWindow::showWindow(GtkMenu *menu, gpointer data){

  //http://ubuntuforums.org/showthread.php?t=2179045

  Q_UNUSED(menu);
  QApplication *self = static_cast<QApplication *>(data);

  QWindow *wind = self->allWindows().at(0);
  if(wind->isVisible()) wind->hide();
  else wind->show();
}


void MainWindow::quitWindow(GtkMenu *menu, gpointer data){

  //change that methode to use the real nice destruction of the window

  Q_UNUSED(menu);
  QApplication *self = static_cast<QApplication *>(data);
  self->quit();
}










