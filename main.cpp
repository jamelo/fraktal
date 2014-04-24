#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KUrl>

int main(int argc, char** argv)
{
    KAboutData aboutData(
        "fractal-viewer",
        0,
        ki18n("Fractal Viewer"),
        "1.0",
        ki18n("Test program."),
        KAboutData::License_GPL,
        ki18n("(c) 2014 Jordan Melo"),
        ki18n("Enables the exploration and rendering of fractals."),
        "http://jmelo.net/",
        "jmelo@uwaterloo.ca"
    );

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[file]", ki18n("Document to open"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    //MainWindow* window = new MainWindow();
    //window->show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count()) {
        //window->openFile(args->url(0).url());
    }

    return app.exec();
}
