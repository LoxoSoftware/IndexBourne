#include "project.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QMessageBox>
#include "mainwindow.h"

#define ORA_VERSION "0.0.6"

bool Project::SaveProject(QString filename)
{
    QuaZip ozip= QuaZip(filename);
    if (!ozip.open(QuaZip::mdAppend))
        return false;

    QuaZipFile ofile= QuaZipFile(&ozip);
    QXmlStreamWriter xstream= QXmlStreamWriter(&ofile);

    QImage merged_image= RenderBitmap();
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
    for (int ifr=0; ifr<this->Frames(); ifr++)
    {
        for (int il=0; il<FrameAt(ifr)->LayerCount(); il++)
        {
            ofile.open(QIODevice::WriteOnly, QuaZipNewInfo(
                "data/frame"+QString::number(ifr)+
                "/layer"+QString::number(il)+".png"
            ));
            FrameAt(ifr)->Layer(il)->save(&ofile, "png", 100);
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
        for (int ifr=0; ifr<this->Frames(); ifr++)
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
    xstream.writeEndElement();
    xstream.writeEndDocument();
    ofile.close();

    ozip.close();

    return true;
}

bool Project::LoadProject(QString filename)
{
    QuaZip izip= QuaZip(filename);
    if (!izip.open(QuaZip::mdUnzip))
        return false;

    QuaZipFile ifile= QuaZipFile(&izip);
    QXmlStreamReader xstream= QXmlStreamReader(&ifile);

    // -- Handle EZGFX specific extensions --
    izip.setCurrentFile("ezgfx.xml");
    if (!ifile.open(QIODevice::ReadOnly))
    {
        izip.close();
        return false;
    }

    // -- --
    ifile.close();

    // -- Parse stack.xml --
    izip.setCurrentFile("stack.xml");
    if (!ifile.open(QIODevice::ReadOnly))
    {
        izip.close();
        return false;
    }
    QByteArray stack_xml= ifile.readAll();
    QBuffer stack_buffer= QBuffer(&stack_xml);
    stack_buffer.open(QIODevice::ReadOnly);
    xstream.setDevice(&stack_buffer);
    ifile.close();

    QStringView elem_name;
    QXmlStreamAttributes elem_attrs;
    QSize new_size= QSize(-1, -1);
    QList<Frame> new_frames;
    palette_t new_palette;
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

                    tframe.PushNewSnapshot(new UndoSnapshot(Undocmd_AddLayer, 0, tframe.Layer(0)));
                    new_frames += tframe;
                }
            }
        }

    }
    stack_buffer.close();

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
    this->SetPalette(new_palette);
    // -- --

    Canvas()->Redraw();
    UiLayerPanel()->Update();
    UiPalettePanel()->Update();

    izip.close();

    return true;
}