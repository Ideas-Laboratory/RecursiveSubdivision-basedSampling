#include "qt_gui.h"
#include <QtWidgets/QApplication>

Param params = { 6,6,4,0.02,0.02,false,false };
//std::vector<int> selected_class_order{ 0,1,2,3,4,5,6,7,8,9 };
std::vector<int> selected_class_order{ 0,6,7,10 };

int main(int argc, char *argv[])
{
	srand(time(NULL));

	QApplication a(argc, argv);
	Qt_GUI w;
	w.show();
	return a.exec();
}
