#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QListWidgetItem>
#include <QGraphicsScene>
#include <QToolButton>
#include <QFuture>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:

	void resizeEvent(QResizeEvent *event);

	void closeEvent(QCloseEvent *event);

	void on_listImages_itemClicked(QListWidgetItem *item);

	void on_listImages_itemDoubleClicked(QListWidgetItem *item);

	void on_listImages_currentRowChanged(int currentRow);

	void on_actionQuit_triggered();

	void on_actionAbout_Qt_triggered();

	void on_actionAbout_triggered();

	void on_actionLoad_Images_triggered();

	void on_actionRemove_Selected_Images_triggered();

	void on_actionRemove_All_Images_triggered();

	void scenesClear();

	void showScaledPixmap();

	void on_actionConvert_Selected_Images_triggered();

	void on_actionConvert_All_Images_triggered();

	void convert();

	void on_actionCancel_Converting_triggered();

	void on_buttonFolder_clicked();

	void on_radioSourceFolder_clicked();

	void on_radioFolder_clicked();


private:
	Ui::MainWindow *ui;

	QStringList fileNamesCurrent, fileNames;
	QVector<int> fileSizes;
	QVector<int> tableColumnWidths;
    QString defaultPath = "C:/Qt/Qt Projects/TEST_IMAGES/images100";
	QGraphicsScene *scenePreview = new QGraphicsScene();
	QGraphicsScene *sceneViewer = new QGraphicsScene();
    QString extension, message;
	QMap<QString, QString> formatMap;
	int quality;
    bool convertAll, cancelConverting;
	QList<int> indexList;
	QList<QToolButton *> allToolButtons;
	int fileCount, fileCounter;
	QString outputDir;
    QString encoderHalicPath, decoderHalicPath;
    quint64 totalInputSize, totalOutputSize, totalTime;
    QFuture<void> future;

};
#endif // MAINWINDOW_H
