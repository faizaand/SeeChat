#include "FL/Fl.H"
#include "FL/Fl_Box.H"
#include "FL/Fl_Window.H"

int main() {
    Fl_Window window(400, 600, "SeeChat Client");
    Fl_Box box(0, 50, 200, 20, "Hello, SeeChat!");
    window.show();
    return Fl::run();
}
