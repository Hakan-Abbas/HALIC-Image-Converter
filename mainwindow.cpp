#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QProcess>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <QImageReader>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(scenePreview);
    ui->graphicsViewViewer->setScene(sceneViewer);

//	QTabBar *tabBar = ui->tabWidget->tabBar();
//	tabBar->tabButton(0, QTabBar::RightSide)->hide();
//	tabBar->tabButton(1, QTabBar::RightSide)->hide();

    ui->labelImageInfo->setVisible(false);

    ////////////////////////
    ui->tableWidget->setColumnWidth(0,460); ui->tableWidget->setColumnWidth(1,80); ui->tableWidget->setColumnWidth(2,120); ui->tableWidget->setColumnWidth(3,120);
    ui->tableWidget->setColumnWidth(4,100); ui->tableWidget->setColumnWidth(5,80); ui->tableWidget->setColumnWidth(6,100);

    tableColumnWidths << 460 << 80 << 120 << 120 << 100 << 80 << 100;

    for (int i = 0; i < 23; i++) ui->tableWidget->insertRow(i);
    ////////////////////////
    allToolButtons = ui->toolBar->findChildren<QToolButton *>();
	allToolButtons.at(6)->setEnabled(false);
	////////////////////////
	formatMap.insert("HALIC", "halic");
	formatMap.insert("AVIF", "avif");
	formatMap.insert("BCM", "bcm");
	formatMap.insert("FOX", "fox");
	formatMap.insert("SIF", "sif");
	formatMap.insert("QIC", "qic");
	formatMap.insert("BIM", "bim");
	formatMap.insert("BMF", "bmf");
	formatMap.insert("FLIC", "flic");
	formatMap.insert("GRALIC", "gralic");
	formatMap.insert("HEIF", "heif");
    formatMap.insert("HLX", "hlx");
	formatMap.insert("JPEG 2000", "jp2");
	formatMap.insert("JPEG LS", "jls");
	formatMap.insert("JPEG XL", "jxl");
	formatMap.insert("KVICK", "kvick");
	formatMap.insert("LEA", "lea");
	formatMap.insert("PNG", "png");
    formatMap.insert("SLIC", "slic");
	formatMap.insert("QOI", "qoi");
	formatMap.insert("QLIC", "qlic");
	formatMap.insert("WEBP", "webp");
	formatMap.insert("WEBP 2", "wp2");
	formatMap.insert("BMP", "bmp");
	formatMap.insert("PPM", "ppm");
	formatMap.insert("PNM", "pnm");
    formatMap.insert("PGM", "pgm");
	formatMap.insert("RAW", "raw");
	formatMap.insert("TGA", "tga");
	formatMap.insert("TIFF", "tif");
    ///////////////////////////////////
    QImageReader::setAllocationLimit(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    int right = 512;
    int left = this->width() - right;
    ui->splitter->setSizes(QList<int>() << left << right);
    //qDebug() << this->width() << left << right;

    float rate = (this->width() - 50) / 1150.0; // Toolbar size = 50 px
    for (int i = 0; i < ui->tableWidget->columnCount(); i++) {
        ui->tableWidget->setColumnWidth(i, tableColumnWidths.at(i) * rate);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!future.isFinished()) {
        event->ignore();
        cancelConverting = true;
        QMessageBox::warning(this, tr("Warning"),"Please wait until active process is finished !");
    }
}

void MainWindow::on_listImages_itemClicked(QListWidgetItem *item)
{
    if (ui->listImages->count() == 1) showScaledPixmap();
}

void MainWindow::on_listImages_itemDoubleClicked(QListWidgetItem *item)
{
    int index = ui->listImages->currentRow();
	QPixmap pixmap(fileNames.at(index));
/*
	QPixmap pixmap;
	if (fileNames.at(index).right(5) == "halic") {
		pixmap.load(fileNames.at(index).left(fileNames.at(index).size() - 5) + "pnm");
	}
	else pixmap.load(fileNames.at(index));
*/
    sceneViewer->clear();
    sceneViewer->addPixmap(pixmap);
    sceneViewer->setSceneRect(pixmap.rect());

    ui->tabWidget->setTabText(1, fileNames.at(index).mid(fileNames.at(index).lastIndexOf("/") + 1));
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::on_listImages_currentRowChanged(int currentRow)
{
    if (ui->listImages->count() == 1 || currentRow == -1) return;
    showScaledPixmap();
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(
        this,
        "About The Author",
		"Hakan ABBAS<br />Computer Science Researcher &amp; Software Specialist<br />"
        "<table>"
        "<tr>"
        "<td align = right>E-Mail&nbsp;&nbsp;&nbsp;&nbsp; :</td>"
        "<td width = 230>&nbsp;<a "
        "href='mailto:abbas.hakan@gmail.com'>abbas.hakan@gmail.com</a></td>"
        "</tr>"
        "<tr>"
        "<td align = right>LinkedIn :</td>"
        "<td>&nbsp;<a "
        "href='https://www.linkedin.com/in/hakan-abbas-178b5852/'>linkedin.com/in/"
        "hakan-abbas-178b5852</a></td>"
        "</tr>"
        "<tr>"
        "<td align = right>Youtube :</td>"
        "<td>&nbsp;<a href='http://www.youtube.com/hakanabbas'>youtube.com/hakanabbas</a></td>"
        "</tr>"
        "</table>");
}

void MainWindow::on_actionRemove_Selected_Images_triggered()
{
    QModelIndexList indexes = ui->listImages->selectionModel()->selectedIndexes();
    QList<int> indexList;
    foreach (QModelIndex index, indexes) {
        indexList.append(index.row());
    }
    std::sort(indexList.begin(), indexList.end(), std::greater<int>());
    //qDebug() << indexList;

    for (int i = 0; i < indexList.size(); i++) {
        delete ui->listImages->takeItem(indexList.at(i));
        fileNames.removeAt(indexList.at(i));
        fileSizes.removeAt(indexList.at(i));
    }

    scenesClear();
}

void MainWindow::on_actionRemove_All_Images_triggered()
{
    fileNames.clear();
    fileSizes.clear();
    ui->listImages->clear();

    scenesClear();
}

void MainWindow::scenesClear()
{
    scenePreview->clear();
    ui->tabWidget->setCurrentIndex(0);
    ui->labelImageInfo->setVisible(false);
}

void MainWindow::showScaledPixmap()
{
    int currentRow = ui->listImages->currentRow();

    QPixmap pixmap;
	pixmap.load(fileNames.at(currentRow));

    //QPixmap pixmap(fileNames.at(currentRow));
    QFileInfo fileInfo(fileNames.at(currentRow));

    ///////////////////////////////////////////
    QString fileName = fileInfo.fileName();
    QString absolutePath = fileInfo.absolutePath();
    if (absolutePath.size() > 60)
        absolutePath = absolutePath.left(28) + "..." + absolutePath.right(28);

    QString lastModified = fileInfo.lastModified().date().toString() + " - "
                           + fileInfo.lastModified().time().toString();

    QString fileSize = QString("%L1").arg(fileSizes.at(currentRow)) + " bytes";
    QString imageFormat = "24 bit RGB";
    QString imageSize = QString::number(pixmap.width()) + " x " + QString::number(pixmap.height())
                   + " px";

    if (pixmap.hasAlpha())
        imageFormat = "32 bit RGBA";

    ///////////////////////////////////////////

    QString imageInfo
        = "<table style='width: 301px;'><tbody><tr><td style='text-align: right; width: "
          "123px;'><strong>Filename :</strong></td><td style='width: 160px;'>"
          + fileName
          + "</td></tr><tr><td "
            "style='text-align: right; width: 123px;'><strong>Directory :</strong></td><td "
            "style='width: "
            "160px;'>"
          + absolutePath
          + "</td></tr><tr><td style='text-align: right; width: 123px;'><strong>Last "
            "Modified :</strong></td><td style='width: 160px;'>"
          + lastModified
          + "</td></tr><tr><td style='text-align: "
            "right; width: 123px;'><strong>File Size :</strong></td><td style='width: "
            "160px;'>"
          + fileSize
          + "</td></tr><tr><td style='text-align: right; width: 123px;'><strong>Resolution "
            ":</strong></td><td style='width: 160px;'>"
          + imageSize
          + "</td></tr><tr><td style='text-align: right; width: 123px;'><strong>Image Format "
            ":</strong></td><td style='width: 160px;'>"
          + imageFormat
          + "</td></tr></tbody></table>";
    ///////////////////////////////////////////

    if (pixmap.width() > 512) pixmap = pixmap.scaledToWidth(512, Qt::SmoothTransformation);

    scenePreview->clear();
    scenePreview->addPixmap(pixmap);
    scenePreview->setSceneRect(pixmap.rect());
    //	ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    ui->labelImageInfo->setText(imageInfo);
    ui->labelImageInfo->setVisible(true);

}

void MainWindow::on_actionLoad_Images_triggered()
{
    fileNamesCurrent = QFileDialog::getOpenFileNames(
        Q_NULLPTR,
        tr("Open Image Files"),
        defaultPath,
    tr("Image Files (*.avif *.bcm *.bim *.bmp *.bmf *.flic *.fox *.gralic *.halic *.heif *.jls *.jp2 *.lea *.jxl *.kvick *.png *.pnm *.ppm *.pgm *.slic *.qic *.qlic *.qoi *.raw *.sif *.tga *.tif *.tiff *.webp *.wp2)"));

    if (fileNamesCurrent.isEmpty())
        return;

    defaultPath = fileNamesCurrent.at(0).left(fileNamesCurrent.at(0).lastIndexOf("/"));

    QPixmap pixmap;
    QFileInfo fileInfo;

    for (int i = 0; i < fileNamesCurrent.size(); i++) {
        bool fileState = false;
        for (int j = 0; j < fileNames.size(); j++) {
            if (fileNamesCurrent.at(i) == fileNames.at(j)) {
                fileState = true;
                break;
            }
        }

	if (!fileState) {

	    fileInfo.setFile(fileNamesCurrent.at(i));
	    fileNames.append(fileNamesCurrent.at(i));
	    fileSizes.append(fileInfo.size());

	    QString shortName = fileInfo.fileName();
	    QString extension = fileInfo.suffix();
	    if (shortName.size() > 20)
		shortName = shortName.left(14) + ".." + shortName.mid(shortName.lastIndexOf("."));

	    bool supportedType = false;
	    if (extension == "bmp" || extension == "png" || extension == "ppm" || extension == "pnm"
		|| extension == "raw" || extension == "tga" || extension == "tiff"
		|| extension == "webp")
		supportedType = true;

	    if (supportedType) {
		pixmap.load(fileNamesCurrent.at(i));
        pixmap = pixmap.scaledToHeight(64, Qt::SmoothTransformation);
		ui->listImages->addItem(new QListWidgetItem(QPixmap(pixmap), shortName));
	    } else {
		ui->listImages->addItem(new QListWidgetItem(
		    QPixmap(
			"C:/Qt/Qt "
			"Projects/ImageViewer3/icons/img.png"),
		    shortName));
	    }
        }
    }

    ui->tabWidget->setCurrentIndex(0);
    //qDebug() << fileNames;

}

void MainWindow::on_actionConvert_Selected_Images_triggered()
{
    if (ui->listImages->selectedItems().size() == 0) {
        QMessageBox::warning(this,
                             tr("Warning"),
							 "Please choose images for conversion!");
        return;
    }

    if (ui->radioFolder->isChecked() && ui->lineEditFolder->text() == "") {
        QMessageBox::warning(this,
                             tr("Warning"),
							 "Please select an \"Output Folder\" from the Options tab!");
        ui->tabWidget->setCurrentIndex(3);
        return;
    }

    convertAll = false;
	cancelConverting = false;

    QModelIndexList indexes = ui->listImages->selectionModel()->selectedIndexes();
    indexList.clear();
    foreach (QModelIndex index, indexes) {
        indexList.append(index.row());
    }
    std::sort(indexList.begin(), indexList.end());
    //	qDebug() << indexList;

	fileCount = indexList.size();
	fileCounter = 0;
	///////////////////////
	extension = ui->comboFormat->currentText();
	extension = formatMap.value(extension);
	///////////////////////
	ui->tableWidget->setRowCount(fileCount);
    ///////////////////////
    for (int i = 0; i < fileCount; i++) {
        QTableWidgetItem *itemFileName = new QTableWidgetItem();
        QTableWidgetItem *itemFormat = new QTableWidgetItem();
        QTableWidgetItem *itemOriginalSize = new QTableWidgetItem();
        QTableWidgetItem *itemConvertedSize = new QTableWidgetItem();
        QTableWidgetItem *itemCompressionRatio = new QTableWidgetItem();
        QTableWidgetItem *itemConversionTime = new QTableWidgetItem();
        QTableWidgetItem *itemState = new QTableWidgetItem();

        itemFileName->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemFormat->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        itemOriginalSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemConvertedSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemCompressionRatio->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        itemConversionTime->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemState->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

        ui->tableWidget->setItem(i, 0, itemFileName);
        ui->tableWidget->setItem(i, 1, itemFormat);
        ui->tableWidget->setItem(i, 2, itemOriginalSize);
        ui->tableWidget->setItem(i, 3, itemConvertedSize);
        ui->tableWidget->setItem(i, 4, itemCompressionRatio);
        ui->tableWidget->setItem(i, 5, itemConversionTime);
        ui->tableWidget->setItem(i, 6, itemState);

        ui->tableWidget->item(i, 0)->setText(fileNames.at(indexList.at(i)));
        ui->tableWidget->item(i, 2)->setText(QString("%L1").arg(fileSizes.at(indexList.at(i))) + " bytes");
        ui->tableWidget->item(i, 6)->setText("Ready");
    }
	/////////////////////

	allToolButtons.at(4)->setEnabled(false);
	allToolButtons.at(5)->setEnabled(false);
	allToolButtons.at(6)->setEnabled(true);

    totalInputSize = 0;
    totalOutputSize = 0;
    totalTime = 0;

    future = QtConcurrent::run([this] { convert(); });
}

void MainWindow::on_actionConvert_All_Images_triggered()
{
	ui->listImages->selectAll();
	on_actionConvert_Selected_Images_triggered();

	/*
    if (ui->listImages->count() == 0) {
        QMessageBox::warning(this,
                             tr("Warning"),
                             "Please add images for conversion!  ");
        return;
    }

    if (ui->radioFolder->isChecked() && ui->lineEditFolder->text() == "") {
        QMessageBox::warning(this,
                             tr("Warning"),
                             "Please select an \"Output Folder\" from the Options tab! ");
        ui->tabWidget->setCurrentIndex(3);
        return;
    }

    convertAll = true;
    cancelConverting = false;

    fileCount = fileNames.size();
    fileCounter = 0;
    extension = ui->comboFormat->currentText();
    extensionIndex = ui->comboFormat->currentIndex();

    ui->tableWidget->setRowCount(fileCount);
    ///////////////////////
    for (int i = 0; i < fileCount; i++) {
        QTableWidgetItem *itemFileName = new QTableWidgetItem();
        QTableWidgetItem *itemOriginalSize = new QTableWidgetItem();
        QTableWidgetItem *itemConvertedSize = new QTableWidgetItem();
        QTableWidgetItem *itemCompressionRatio = new QTableWidgetItem();
        QTableWidgetItem *itemConversionTime = new QTableWidgetItem();
        QTableWidgetItem *itemState = new QTableWidgetItem();

        itemFileName->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemOriginalSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemConvertedSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemCompressionRatio->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        itemConversionTime->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemState->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

        ui->tableWidget->setItem(i, 0, itemFileName);
        ui->tableWidget->setItem(i, 1, itemOriginalSize);
        ui->tableWidget->setItem(i, 2, itemConvertedSize);
        ui->tableWidget->setItem(i, 3, itemCompressionRatio);
        ui->tableWidget->setItem(i, 4, itemConversionTime);
        ui->tableWidget->setItem(i, 5, itemState);

        ui->tableWidget->item(i, 0)->setText(fileNames.at(i));
        ui->tableWidget->item(i, 1)->setText(QString("%L1").arg(fileSizes.at(i)) + " bytes");
        ui->tableWidget->item(i, 5)->setText("Ready");
    }
    ///////////////////////

    allToolButtons.at(4)->setEnabled(false);
    allToolButtons.at(5)->setEnabled(false);
    allToolButtons.at(6)->setEnabled(true);

    totalInputSize = 0;
    totalOutputSize = 0;
	totalTime = 0;

	future = QtConcurrent::run(this, &MainWindow::convert);
*/
}

void MainWindow::convert()
{
    /////////////////////////////
    if (cancelConverting) {
        allToolButtons.at(4)->setEnabled(true);
        allToolButtons.at(5)->setEnabled(true);
        allToolButtons.at(6)->setEnabled(false);
        return;
    }

    ui->tabWidget->setCurrentIndex(2);

    ///////////////////////
    ui->tableWidget->item(fileCounter, 6)->setText("Converting...");
    for (int i = 0; i <= 6; i++) {
    if (fileCounter % 2 == 0)
            ui->tableWidget->item(fileCounter, i)->setBackground(QColor(0, 116, 139));
        else
            ui->tableWidget->item(fileCounter, i)->setBackground(QColor(0, 83, 99));
    }
    ///////////////////////

    QString fileInput, fileOutput, fileExtension;
    quint64 fileSizeInput;

    if (convertAll) {
        fileInput = fileNames.at(fileCounter);
        fileSizeInput = fileSizes.at(fileCounter);
    } else {
        fileInput = fileNames.at(indexList.at(fileCounter));
        fileSizeInput = fileSizes.at(indexList.at(fileCounter));
    }

    QFileInfo fileInfo(fileInput);
    fileExtension = fileInfo.suffix();
//	qDebug() << fileExtension;

    QPixmap pixmap;
    pixmap.load(fileInput);

    ///////////////////////
    if (ui->radioSourceFolder->isChecked())
	fileOutput = fileInfo.absolutePath() + "/" + fileInfo.baseName() + "." + extension;
    else {
	fileOutput = outputDir + "/" + fileInfo.baseName() + "." + extension;
    }
    ///////////////////////

	///////////////////////////////////////////////////
    QProcess process;
	QStringList args;
	///////////////////////////////////////////////////

	QElapsedTimer timer;
	timer.start();

	/////////////////////////////
	if (extension == "halic") {
		if (ui->radioHalicRapid->isChecked()) {
			args << fileInput << fileOutput << "-pnm";
            process.execute("CODECS/HALIC/Halic-Rapid-Encoder-1.0b.exe", args);
		} else if (ui->radioHalic04->isChecked()) {
			args << fileInput << fileOutput << "-pnm";
            process.execute("CODECS/HALIC/Halic-Balanced-Encoder-v.0.4.exe", args);
        } else if (ui->radioHalic07->isChecked()) {
            QString cmd = "CODECS/HALIC/HALIC_ENCODE_V.0.7." + QString::number(ui->spin_Halic->value());

            if (ui->checkBoxMT->isChecked()) cmd += "_MT";
            else cmd += "_ST";

            if (ui->checkBoxFast->isChecked()) cmd += "_FAST";
            cmd += "_AVX.exe";

            if (ui->checkBoxMT->isChecked()) {
                args << fileInput << fileOutput << "-mt";
                process.execute(cmd, args);
            }
            else {
                args << fileInput << fileOutput;
                process.execute(cmd, args);
            }
		}
	} else if (extension == "bmf") {
		args << fileInput.replace('/','\\'); // "-s"
        process.execute("CODECS/BMF/BMF.exe", args);
	} else if (extension == "qlic") {
		args << "c" << fileOutput << fileInput;
        process.execute("CODECS/QLIC/QLIC2.exe", args);

	} else if (extension == "qic") {
		args << "c" << fileOutput << fileInput;
        process.execute("CODECS/QIC/QIC.exe", args);
	} else if (extension == "fox") {
		args << fileInput << fileOutput;
        process.execute("CODECS/FOX/cfox.exe", args);
	} else if (extension == "sif") {
		args << "c" << fileInput << fileOutput;
        process.execute("CODECS/SIF/SIF.exe", args);
    } else if (extension == "slic") {
        args << "e" << fileInput << fileOutput;
        process.execute("CODECS/SLIC/slic2.exe", args);
	} else if (extension == "bcm") {
		args << "c2000" << fileInput << fileOutput;
        process.execute("CODECS/BCM/bcm.exe", args);
	} else if (extension == "kvick") {
        args << "c" << "i" << fileOutput << fileInput;
        process.execute("CODECS/KVICK/kvick.exe", args);
    } else if (extension == "hlx") {
        args << "c" << "i" << fileOutput << fileInput;
        process.execute("CODECS/HLX/hlx.exe", args);
	} else if (extension == "bim") {
		args << "c" << fileInput << fileOutput;
        process.execute("CODECS/BIM/bim.exe", args);
	} else if (extension == "flic") {
		args << "c" << fileOutput << fileInput;
        process.execute("CODECS/FLIC/FLIC.exe", args);
	} else if (extension == "gralic") {
		args << "c" << fileOutput << fileInput;
        process.execute("CODECS/GRALIC/Gralic111d.exe", args);
	} else if (extension == "png") {
		if (fileExtension == "webp") {
			args << "-o" << fileOutput << fileInput;
            process.execute("CODECS/WEBP/dwebp.exe", args);
		} else if (fileExtension == "jp2") {
			args << "-i" << fileInput << "-o" << fileOutput;
            process.execute("CODECS/JP2/opj_decompress.exe", args);
		} else if (fileExtension == "jxl") {
			args << fileInput << fileOutput << "--num_threads"
				 << "8";
            process.execute("CODECS/JXL/djxl.exe", args);
		} else if (fileExtension == "avif") {
			args << "--jobs"
				 << "8" << fileInput << fileOutput;
            process.execute("CODECS/AVIF/avifdec.exe", args);
		} else if (fileExtension == "heif") {
			args << fileInput << fileOutput;
            process.execute("CODECS/HEIF/heif-convert.exe", args);
		} else if (fileExtension == "qoi") {
			args << fileInput << fileOutput;
            process.execute("CODECS/QOI/qoiconv.exe", args);
		}
		else
			pixmap.save(fileOutput, "PNG", (90 - ui->spinPNG->value() * 10));
	} else if (extension == "webp") {
        //args << "-lossless" << "-m" << QString::number(ui->spinWebp_m->value()) << fileInput << "-o" << fileOutput;
        args << "-z" << QString::number(ui->spinWebp_z->value()) << fileInput << "-o" << fileOutput;
        //args << "-m" << "6" << "-q" << "75" << fileInput << "-o" << fileOutput;
        //args << "-m" << "6" << "-near_lossless" << "30" << fileInput << "-o" << fileOutput;
        process.execute("CODECS/WEBP/cwebp.exe", args);
	} else if (extension == "wp2") {
		args << "-q" << "100" << "-mt" << "1" << fileInput << "-o" << fileOutput;
        process.execute("CODECS/WEBP2/cwp2.exe", args);
	} else if (extension == "lea") {
		args << fileInput << fileOutput;
        process.execute("CODECS/LEA/clea.exe", args);
    } else if (extension == "jxl") {
        //args << fileInput << fileOutput << "1" << "1" << "1"; // fast_lossless
        args << fileInput << fileOutput << "-q" << "100" << "-e" << QString::number(ui->spinJXL->value()) << "--num_threads" << QString::number(ui->spinJXL_Thread->value());
        QString dir;
        if (ui->radio_Jxl_9->isChecked()) dir = "CODECS/JXL/0.9/";
        else if(ui->radio_Jxl_10->isChecked()) dir = "CODECS/JXL/0.10/";
        process.execute(dir + "cjxl.exe", args);
    } else if (extension == "jls") {
		args << "-o" + fileOutput << fileInput;
        process.execute("CODECS/JLS/locoe.exe", args);
	} else if (extension == "jp2") {
		args << "-i" << fileInput << "-o" << fileOutput;
        process.execute("CODECS/JP2/opj_compress.exe", args);
	} else if (extension == "qoi") {
		args << fileInput << fileOutput;
        process.execute("CODECS/QOI/qoiconv.exe", args);
	} else if (extension == "avif") {
		args << "--speed" << "6" << "--jobs" << "8" << "--lossless" << fileInput << fileOutput;
		//			args << "--speed" << "9" << "--jobs" << "8" << "--min" << "0" << "--max" << "50" << fileInput << fileOutput;
        process.execute("CODECS/AVIF/avifenc.exe", args);
	} else if (extension == "heif") {
		args << "-p"
			 << "lossless=true"
			 << "-p"
			 << "chroma=444"
			 << "--matrix_coefficients=0"
			 << "-o" << fileOutput << fileInput;
        process.execute("CODECS/HEIF/heif-enc.exe", args);
    } else if (extension == "pgm") {
        if (fileExtension == "halic") {
            QString cmd = "CODECS/HALIC/HALIC_DECODE_V.0.7." + QString::number(ui->spin_Halic->value());

            if (ui->checkBoxMT->isChecked()) cmd += "_MT";
            else cmd += "_ST";

            if (ui->checkBoxFast->isChecked()) cmd += "_FAST";
            cmd += "_AVX.exe";

            if (ui->checkBoxMT->isChecked()) {
                args << fileInput << fileOutput << "-mt";
                process.execute(cmd, args);
            }
            else {
                args << fileInput << fileOutput;
                process.execute(cmd, args);
            }
        }
        else if (fileExtension == "jxl") {
            QString dir;
            if (ui->radio_Jxl_9->isChecked()) dir = "CODECS/JXL/0.9/";
            else if(ui->radio_Jxl_10->isChecked()) dir = "CODECS/JXL/0.10/";
            args << fileInput << fileOutput << "--num_threads" << QString::number(ui->spinJXL_Thread->value());
            process.execute(dir + "djxl.exe", args);
        }
    } else if (extension == "bmp") {
		if (fileExtension == "webp") {
			args << "-bmp"
				 << "-o" << fileOutput << fileInput;
            process.execute("CODECS/WEBP/dwebp.exe", args);
		} else if (fileExtension == "jp2") {
			args << "-i" << fileInput << "-o" << fileOutput;
            process.execute("CODECS/JP2/opj_decompress.exe", args);
		} else if (fileExtension == "jxl") {
			args << fileInput << fileOutput;
            process.execute("CODECS/JXL/0.8.1/djxl.exe", args);
		} else if (fileExtension == "bmf") {
			args << "-bmp" << fileInput.replace('/', '\\') << "-o" << fileOutput.replace('/', '\\');
            process.execute("CODECS/BMF/BMF.exe", args);
		} else if (fileExtension == "bim") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/BIM/bim.exe", args);
		} else
			pixmap.save(fileOutput, "BMP");
	} else if (extension == "ppm") {
		if (fileExtension == "halic") {
			if (ui->radioHalicRapid->isChecked()) {
				args << fileInput << fileOutput << "-pnm";
                process.execute("CODECS/HALIC/Halic-Rapid-Decoder-1.0b.exe", args);
			} else if (ui->radioHalic04->isChecked()) {
				args << fileInput << fileOutput << "-pnm";
                process.execute("CODECS/HALIC/Halic-Balanced-Decoder-v.0.4.exe", args);
            } else if (ui->radioHalic07->isChecked()) {
                QString cmd = "CODECS/HALIC/HALIC_DECODE_V.0.7." + QString::number(ui->spin_Halic->value());

                if (ui->checkBoxMT->isChecked()) cmd += "_MT";
                else cmd += "_ST";

                if (ui->checkBoxFast->isChecked()) cmd += "_FAST";
                cmd += "_AVX.exe";

                if (ui->checkBoxMT->isChecked()) {
                    args << fileInput << fileOutput << "-mt";
                    process.execute(cmd, args);
                }
                else {
                    args << fileInput << fileOutput;
                    process.execute(cmd, args);
                }
			}
		} else if (fileExtension == "webp") {
			args << "-ppm"
				 << "-o" << fileOutput << fileInput;
            process.execute("CODECS/WEBP/dwebp.exe", args);
		} else if (fileExtension == "wp2") {
			args << "-ppm" << "-mt" << "8" << "-o" << fileOutput << fileInput;
            process.execute("CODECS/WEBP2/dwp2.exe", args);
		} else if (fileExtension == "jp2") {
			args << "-i" << fileInput << "-o" << fileOutput;
            process.execute("CODECS/JP2/opj_decompress.exe", args);
		} else if (fileExtension == "jxl") {
            QString dir;
            if (ui->radio_Jxl_9->isChecked()) dir = "CODECS/JXL/0.9/";
            else if(ui->radio_Jxl_10->isChecked()) dir = "CODECS/JXL/0.10/";
            args << fileInput << fileOutput << "--num_threads" << QString::number(ui->spinJXL_Thread->value());
            process.execute(dir + "djxl.exe", args);
		} else if (fileExtension == "jls") {
			args << "-P"
				 << "-o" + fileOutput << fileInput;
            process.execute("CODECS/JLS/locod.exe", args);
		} else if (fileExtension == "qlic") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/QLIC/QLIC2.exe", args);
		} else if (fileExtension == "flic") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/FLIC/FLIC.exe", args);
		} else if (fileExtension == "qic") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/QIC/QIC.exe", args);
		} else if (fileExtension == "sif") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/SIF/SIF.exe", args);
		} else if (fileExtension == "fox") {
			args << fileInput << fileOutput;
            process.execute("CODECS/FOX/dfox.exe", args);
		} else if (fileExtension == "gralic") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/GRALIC/Gralic111d.exe", args);
		} else if (fileExtension == "lea") {
			args << fileInput << fileOutput;
            process.execute("CODECS/LEA/dlea.exe", args);
		} else if (fileExtension == "bim") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/BIM/bim.exe", args);
	} else pixmap.save(fileOutput, "PPM");
	} else if (extension == "pnm") {
    if (fileExtension == "halic") {
			if (ui->radioHalicRapid->isChecked()) {
				args << fileInput << fileOutput << "-pnm";
                process.execute("CODECS/HALIC/Halic-Rapid-Decoder-1.0b.exe", args);
			} else if (ui->radioHalic04->isChecked()) {
				args << fileInput << fileOutput << "-pnm";
                process.execute("CODECS/HALIC/Halic-Balanced-Decoder-v.0.4.exe", args);
            } else if (ui->radioHalic07->isChecked()) {
                QString cmd = "CODECS/HALIC/HALIC_DECODE_V.0.7." + QString::number(ui->spin_Halic->value());

                if (ui->checkBoxMT->isChecked()) cmd += "_MT";
                else cmd += "_ST";

                if (ui->checkBoxFast->isChecked()) cmd += "_FAST";
                cmd += "_AVX.exe";

                if (ui->checkBoxMT->isChecked()) {
                    args << fileInput << fileOutput << "-mt";
                    process.execute(cmd, args);
                }
                else {
                    args << fileInput << fileOutput;
                    process.execute(cmd, args);
                }
            }
    } else if (fileExtension == "bim") {
            args << "d" << fileInput << fileOutput;
            process.execute("CODECS/BIM/bim.exe", args);
	} else if (fileExtension == "webp") {
			args << "-ppm"
			     << "-o" << fileOutput << fileInput;
            process.execute("CODECS/WEBP/dwebp.exe", args);
	} else if (fileExtension == "wp2") {
			args << "-ppm" << "-mt" << "8" << "-o" << fileOutput << fileInput;
            process.execute("CODECS/WEBP2/dwp2.exe", args);
	} else if (fileExtension == "jp2") {
			args << "-i" << fileInput << "-o" << fileOutput;
            process.execute("CODECS/JP2/opj_decompress.exe", args);
	} else if (fileExtension == "jls") {
			args << "-P"
			     << "-o" + fileOutput << fileInput;
            process.execute("CODECS/JLS/locod.exe", args);
	} else if (fileExtension == "qlic") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/QLIC/QLIC2.exe", args);
	} else if (fileExtension == "flic") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/FLIC/FLIC.exe", args);
	} else if (fileExtension == "qic") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/QIC/QIC.exe", args);
	} else if (fileExtension == "sif") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/SIF/SIF.exe", args);
    } else if (fileExtension == "slic") {
            args << "d" << fileInput << fileOutput;
            process.execute("CODECS/SLIC/slic2.exe", args);
	} else if (fileExtension == "fox") {
			args << fileInput << fileOutput;
            process.execute("CODECS/FOX/dfox.exe", args);
	} else if (fileExtension == "gralic") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/GRALIC/Gralic111d.exe", args);
	} else if (fileExtension == "bcm") {
			args << "d" << fileInput << fileOutput;
            process.execute("CODECS/BCM/bcm.exe", args);
	} else if (fileExtension == "kvick") {
			args << "d" << "i" << fileInput << fileOutput;
            process.execute("CODECS/KVICK/kvick.exe", args);
    } else if (fileExtension == "hlx") {
            args << "d" << "i" << fileInput << fileOutput;
            process.execute("CODECS/HLX/hlx.exe", args);
	} else if (fileExtension == "lea") {
			args << fileInput << fileOutput;
            process.execute("CODECS/LEA/dlea.exe", args);
	} else if (fileExtension == "bmf") {
			args << "-pnm" << fileInput.replace('/','\\') << "-o" << fileOutput.replace('/','\\');
            process.execute("CODECS/BMF/BMF.exe", args);
	} else pixmap.save(fileOutput, "PPM");
	} else if (extension == "tiff") {
		if (fileExtension == "webp") {
			args << "-tiff"
				 << "-o" << fileOutput << fileInput;
            process.execute("CODECS/WEBP/dwebp.exe", args);
		} else if (fileExtension == "jp2") {
			args << "-i" << fileInput << "-o" << fileOutput;
            process.execute("CODECS/JP2/opj_decompress.exe", args);
		} else
			pixmap.save(fileOutput, "TIFF");
	}
	/////////////////////////////

    unsigned int elapsedTime = timer.elapsed();

    fileInfo.setFile(fileOutput);
    //QString fileSize = ui->tableWidget->item(fileCounter, 1)->text();

    double compressionRatio = ((double)fileInfo.size() / fileSizeInput) * 100.0;
//	qDebug() << fileInfo.size() << fileSize << compressionRatio;

    /////////////////////////////
    ui->tableWidget->scrollToItem(ui->tableWidget->item(fileCounter, 0));
    /////////////////////////////

    ui->tableWidget->item(fileCounter, 1)->setText(QString(formatMap.key(extension)));
    ui->tableWidget->item(fileCounter, 3)->setText(QString("%L1").arg(fileInfo.size()) + " bytes");
    ui->tableWidget->item(fileCounter, 4)->setText(QString::number(compressionRatio) + " %");
    ui->tableWidget->item(fileCounter, 5)->setText(QString::number(elapsedTime) + " ms");
    ui->tableWidget->item(fileCounter, 6)->setText("Converted!");
    /////////////////////////////

    totalInputSize += fileSizeInput;
    totalOutputSize += fileInfo.size();
    totalTime += elapsedTime;

    message = "Total Input Size: " + QString("%L1").arg(totalInputSize) + " bytes | "
            + "Total Output Size: " + QString("%L1").arg(totalOutputSize) + " bytes | "
            + "Ratio: " + QString::number(((float) totalOutputSize / totalInputSize) * 100) + " % | "
            + "Total Time: " + QString::number(totalTime) + " ms";
    ui->statusbar->showMessage(message);

    updateGeometry();
    /////////////////////////////
    fileCounter++;
    if (fileCounter == fileCount) {
        allToolButtons.at(4)->setEnabled(true);
        allToolButtons.at(5)->setEnabled(true);
        allToolButtons.at(6)->setEnabled(false);
        return;
    }
    /////////////////////////////
    convert();
    /////////////////////////////
}

void MainWindow::on_actionCancel_Converting_triggered()
{
    if (!future.isFinished()) {
        cancelConverting = true;
        QMessageBox::warning(this, tr("Warning"), "Please wait until active process is finished !");
    }
}

void MainWindow::on_buttonFolder_clicked()
{
    outputDir = QFileDialog::getExistingDirectory(this,
                                                  tr("Select Directory"),
                                                  defaultPath,
                                                  QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);
    ui->lineEditFolder->setText(outputDir);
    defaultPath = outputDir;
}

void MainWindow::on_radioSourceFolder_clicked()
{
    ui->lineEditFolder->setEnabled(false);
    ui->buttonFolder->setEnabled(false);
}

void MainWindow::on_radioFolder_clicked()
{
    ui->lineEditFolder->setEnabled(true);
    ui->buttonFolder->setEnabled(true);
}
