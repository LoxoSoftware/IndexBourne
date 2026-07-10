#include "project.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QMessageBox>
#include <QDir>
#include "mainwindow.h"

#define ORA_VERSION     "0.0.6"

#define PAL_HEADER_SZ   16

QByteArray IntToBigEndian(long n, int size= 4)
{
    QByteArray result;
    for (int i=size-1; i>=0; i--)
        result += (n>>(8*i))&0xFF;
    return result;
}

QByteArray IntToLittleEndian(long n, int size= 4)
{
    QByteArray result;
    for (int i=0; i<size; i++)
        result += (n>>(8*i))&0xFF;
    return result;
}

bool Project::SaveProject(QString filename)
{
    QuaZip ozip= QuaZip(filename);
    if (!ozip.open(QuaZip::mdAppend))
        return false;

    if (FrameCount() < 1)
    {
        QMessageBox::critical(this->main_window, "Save error", "A project must have at least one frame");
        return false;
    }

    QDir project_dir= QDir(filename.first(filename.lastIndexOf('/')));

    QuaZipFile ofile= QuaZipFile(&ozip);
    QXmlStreamWriter xstream= QXmlStreamWriter(&ofile);

    QImage merged_image= CurrentFrame()->RenderBitmap();
    merged_image.convertTo(QImage::Format_ARGB32);

    ofile.open(QIODevice::WriteOnly, QuaZipNewInfo("mimetype"));
    ofile.write("image/openraster");
    ofile.close();

    ofile.open(QIODevice::WriteOnly, QuaZipNewInfo("mergedimage.png"));
    merged_image.save(&ofile, "png");
    ofile.close();

    ofile.open(QIODevice::WriteOnly, QuaZipNewInfo("Thumbnails/thumbnail.png"));
    merged_image.transformed(QTransform::fromScale(
        merged_image.width()>256 ? 256.0/(double)merged_image.width() : 1.0,
        merged_image.height()>256 ? 256.0/(double)merged_image.height() : 1.0
    )).save(&ofile, "png");
    ofile.close();

    //Export all layers' image data
    for (int ifr=0; ifr<this->FrameCount(); ifr++)
    {
        for (int il=0; il<FrameAt(ifr)->LayerCount(); il++)
        {
            ofile.open(QIODevice::WriteOnly, QuaZipNewInfo(
                "data/frame"+QString::number(ifr)+
                "/layer"+QString::number(il)+".png"
            ));
            FrameAt(ifr)->LayerAt(il)->image.save(&ofile, "png", 100);
            ofile.close();
        }
    }

    ofile.open(QIODevice::WriteOnly, QuaZipNewInfo("stack.xml"));
    xstream.setAutoFormatting(true);
    xstream.setAutoFormattingIndent(true);
    xstream.writeStartDocument();
    xstream.writeComment("EZGFX project file");
    xstream.writeStartElement("", "image");
    xstream.writeAttribute("", "version", ORA_VERSION);
    xstream.writeAttribute("", "w", QString::number(this->image_size.width()));
    xstream.writeAttribute("", "h", QString::number(this->image_size.height()));
        xstream.writeStartElement("", "stack"); //Root stack BEGIN
        for (int ifr=0; ifr<this->FrameCount(); ifr++)
        {
            xstream.writeStartElement("", "stack"); //Frame stack BEGIN
            xstream.writeAttribute("", "name", "frame"+QString::number(ifr));
            for (int il=FrameAt(ifr)->LayerCount()-1; il>=0; il--)
            {
                xstream.writeStartElement("", "layer");
                xstream.writeAttribute("", "name", "layer"+QString::number(il));
                xstream.writeAttribute("", "visibility", "visible");
                xstream.writeAttribute("", "src",
                    "data/frame"+QString::number(ifr)+
                    "/layer"+QString::number(il)+".png");
                xstream.writeEndElement();
            }
            xstream.writeEndElement(); //Frame stack END
        }
        xstream.writeEndElement(); //Root stack END
    xstream.writeEndElement();
    xstream.writeEndDocument();
    ofile.close();

    ofile.open(QIODevice::WriteOnly, QuaZipNewInfo("ezgfx.xml"));
    xstream.writeStartDocument();
    xstream.writeStartElement("", "ezgfx");
    xstream.writeAttribute("", "version", "0.0.1");
    xstream.writeTextElement("", "shared_palette", project_dir.relativeFilePath(shared_palette_filename));
    xstream.writeEndElement();
    xstream.writeEndElement();
    xstream.writeEndDocument();
    ofile.close();

    if (shared_palette_filename != "")
        SavePalette(shared_palette_filename);

    ozip.close();

    return true;
}

bool Project::LoadProject(QString filename)
{
    QuaZip izip= QuaZip(filename);
    if (!izip.open(QuaZip::mdUnzip))
        return false;

    QuaZipFile ifile= QuaZipFile(&izip);
    QXmlStreamReader xstream;
    QByteArray txml;
    QBuffer buffer= QBuffer(&txml);
    QDir project_dir= QDir(filename.first(filename.lastIndexOf('/')));
    QStringView elem_name;
    QXmlStreamAttributes elem_attrs;

    QSize new_size= QSize(-1, -1);
    QList<Frame> new_frames;
    palette_t new_palette;
    QString new_shared_pal= "";

    buffer.open(QIODevice::ReadOnly);
    xstream.setDevice(&buffer);

    // -- Handle EZGFX specific extensions --
    izip.setCurrentFile("ezgfx.xml");
    if (!ifile.open(QIODevice::ReadOnly))
    {
        izip.close();
        return false;
    }
    txml= ifile.readAll();
    ifile.close();

    while (xstream.readNextStartElement())
    {
        elem_name= xstream.name();
        elem_attrs= xstream.attributes();

        if (elem_name == "ezgfx")
        {
            while (xstream.readNextStartElement())
            {
                elem_name= xstream.name();
                elem_attrs= xstream.attributes();

                if (elem_name == "shared_palette")
                {
                    new_shared_pal= project_dir.absoluteFilePath(xstream.readElementText());
                    continue;
                }
            }

            continue;
        }
    }

    // -- Parse stack.xml --
    izip.setCurrentFile("stack.xml");
    if (!ifile.open(QIODevice::ReadOnly))
    {
        izip.close();
        return false;
    }
    txml= ifile.readAll();
    buffer.seek(0);
    xstream.setDevice(&buffer);
    ifile.close();

    while (xstream.readNextStartElement())
    {
        elem_name= xstream.name();
        elem_attrs= xstream.attributes();

        if (elem_name == "image")
        {
            if (elem_attrs.hasAttribute("w") && elem_attrs.hasAttribute("h"))
                new_size= QSize(elem_attrs.value("w").toInt(), elem_attrs.value("h").toInt());
        }

        if (elem_name == "stack")
        {
            //Parse root stack
            QStringView tname;

            while (xstream.readNextStartElement())
            {
                //Parse each frame stack
                tname= xstream.name();

                if (tname == "stack")
                {
                    QStringView ttname;
                    QXmlStreamAttributes ttattrs;

                    Frame tframe= Frame(this, new_size); //If the "image" tag has not been read
                                                         //  this won't work

                    while (xstream.readNextStartElement())
                    {
                        ttname= xstream.name();
                        ttattrs= xstream.attributes();

                        if (ttname == "layer")
                        {
                            if (!ttattrs.hasAttribute("src"))
                                continue;

                            izip.setCurrentFile(ttattrs.value("src").toString());

                            if (!ifile.open(QIODevice::ReadOnly))
                            {
                                //Layer image not found
                                izip.close();
                                return false;
                            }

                            QImage new_layer;
                            new_layer.load(&ifile, "png");

                            if (new_layer.format() != QImage::Format_Indexed8 || new_layer.colorCount() < 256)
                            {
                                //Layer image is invalid
                                izip.close();
                                return false;
                            }

                            *tframe.InsertLayerAt(0, false)= new_layer;
                            new_palette= new_layer.colorTable();

                            ifile.close();
                        }

                        if (xstream.hasError())
                        {
                            QMessageBox::critical(this->main_window, "XML Parse error", xstream.errorString());
                            izip.close();
                            return false;
                        }

                        xstream.readNextStartElement(); //Ignore the element end
                    }

                    tframe.ClearHistory();
                    new_frames += tframe;
                }
            }
        }
    }

    buffer.close();

    if (new_size == QSize(-1, -1))
    {
        izip.close();
        return false;
    }

    this->timeline.clear();
    foreach (Frame frame, new_frames)
        this->timeline.append(frame);
    this->image_size= new_size;
    this->current_frame= 0;
    this->current_layer= 0;
    this->Canvas()->SetFrame(CurrentFrame());
    SetPalette(new_palette);
    SetSharedPalette(new_shared_pal);
    // -- --

    Canvas()->DiscardFloatingLayer();
    Canvas()->Redraw();
    UiLayerPanel()->Update();
    UiPalettePanel()->Update();

    izip.close();

    return true;
}

bool Project::SavePalette(QString filename, bool interactive)
{
    QFile ofile= QFile(filename);

    if (!ofile.open(QFile::WriteOnly))
    {
        if (interactive)
            QMessageBox::critical(this->main_window, "Error saving palette", "Cannot open output file for writing");
        return false;
    }

    //We are always exporting 256 colors, no more, no less.
    uint32_t pal_sz= PALETTE_W*PALETTE_H*4; //RGBA

    //Write header
    ofile.write("RIFF");
    ofile.write(IntToLittleEndian(PAL_HEADER_SZ+pal_sz, 4));
    ofile.write("PAL data");
    ofile.write(IntToLittleEndian(PAL_HEADER_SZ-12+pal_sz));
    ofile.write(IntToLittleEndian(0x0300, 2));
    ofile.write(IntToLittleEndian(PALETTE_W*PALETTE_H, 2));
    //Write colors
    foreach(QRgb col, Palette())
    {
        ofile.write(IntToBigEndian(col&0x00FFFFFF, 3));  //RGB
        ofile.write(IntToBigEndian(255, 1));             //A
    }

    ofile.close();

    return true;
}

bool Project::LoadPalette(QString filename, bool interactive)
{
    if (filename.endsWith(".png", Qt::CaseInsensitive) || filename.endsWith(".bmp", Qt::CaseInsensitive))
    {
        QImage timg= QImage(filename);
        if (timg.isNull() || timg.colorCount() <= 0)
            return false;

        SetPalette(timg.colorTable(), true);

        return true;
    }

    QFile ifile= QFile(filename);

    if (!ifile.open(QFile::ReadOnly))
    {
        if (interactive)
            QMessageBox::critical(this->main_window, "Error loading palette", "Cannot open input file for reading");
        return false;
    }

    palette_t new_palette;
    QByteArray tstr= "";
    int color_number= 0;

    //Parse header
    tstr= ifile.read(4);
    if (tstr != "RIFF")
        goto load_error;
    ifile.read(4);
    tstr= ifile.read(8);
    if (tstr != "PAL data")
        goto load_error;
    ifile.read(6);
    color_number= ((uint8_t)ifile.read(1)[0])|((uint8_t)ifile.read(1)[0]<<8);
    //Load colors
    for (int ic=0; ic<color_number; ic++)
    {
        tstr= ifile.read(4);
        new_palette += qRgb((uint8_t)tstr[0], (uint8_t)tstr[1], (uint8_t)tstr[2]);
    }

    ifile.close();

    SetPalette(new_palette, true);

    return true;

load_error:
    if (interactive)
        QMessageBox::critical(this->main_window, "Error loading palette", "Parse error");
    return false;
}
