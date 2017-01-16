

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = bicycle_gui
DEPENDPATH += .
INCLUDEPATH += .


#素材库
RESOURCES = bicycle.qrc

#第三方静态库

#libsqlite3.a
unix:!macx: LIBS += -L$$PWD/ -lsqlite3

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/libsqlite3.a

#libgui.a
unix:!macx: LIBS += -L$$PWD/ -lgui

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/libgui.a

#libwireless.a
unix:!macx: LIBS += -L$$PWD/ -lwireless

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/libwireless.a

#liblogdb.a
unix:!macx: LIBS += -L$$PWD/ -llogdb

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/liblogdb.a

#libgeneral.a
unix:!macx: LIBS += -L$$PWD/ -lgeneral

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/libgeneral.a

#liblnt.a
unix:!macx: LIBS += -L$$PWD/ -llnt

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/liblnt.a

#libserialnumber.a
unix:!macx: LIBS += -L$$PWD/ -lserialnumber

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/libserialnumber.a

#librecords.a
unix:!macx: LIBS += -L$$PWD/ -lrecords

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/librecords.a

#libqrencode.a  //网点二维码
unix:!macx: LIBS += -L$$PWD/ -lqrencode

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/libqrencode.a




#头文件
HEADERS += bicycle_dlg.h  \
		   manage_dlg.h  \
		   basic_info_dlg.h  	\
		   ndev_config_dlg.h \
		   sae_register_dlg.h   \
		   sae_info_dlg.h   \
		   sae_status_dlg.h \
		   wireless_dlg.h    \
		   info_query_dlg.h \
		   ticket_info_query_dlg.h \
		   device_info_dlg.h  \
		   debug_tools_dlg.h  \
		   operation_guide_dlg.h \
		   exception_handle_dlg.h \
		   member_service_dlg.h \
		   member_info_query_dlg.h \
		   rent_info_query_dlg.h \
		   on_off_card_service_dlg.h \
		   on_off_card_task.h  \
		   thread_task_handle.h \
		   thread_retrans_handle.h \
		   delay_return_bicycle_dlg.h \
		   config.h \
		   helper.h \  
    recharge_service_dlg.h \
    member_on_card_service_dlg.h \
    delay_return_bicycle_dlg.h \
    qrcode.h \
    myqrencode.h \
    nearby_site_info_query_dlg.h \
    gui_header.h 






#源文件
SOURCES += main.cpp \
		   bicycle_dlg.cpp  \
		   manage_dlg.cpp  \
		   basic_info_dlg.cpp \
		   ndev_config_dlg.cpp \
		   sae_register_dlg.cpp \
		   sae_info_dlg.cpp \
		   sae_status_dlg.cpp \
		   wireless_dlg.cpp \
		   info_query_dlg.cpp \
		   ticket_info_query_dlg.cpp \
		   device_info_dlg.cpp  \
		   debug_tools_dlg.cpp \	
		   operation_guide_dlg.cpp \
		   exception_handle_dlg.cpp \
		   member_service_dlg.cpp \
		   member_info_query_dlg.cpp \
		   rent_info_query_dlg.cpp \
		   on_off_card_service_dlg.cpp \
		   on_off_card_task.cpp  \
		   thread_task_handle.cpp \
		   thread_retrans_handle.cpp \
    recharge_service.cpp \
    member_on_card_service_dlg.cpp \
    delay_return_bicycle_dlg.cpp \
    qrcode.cpp \
    myqrencode.cpp \
    nearby_site_info_query_dlg.cpp




#UI文件
FORMS += ui/bicycle_ui.ui \
		 ui/manage_ui.ui  \
		 ui/basic_info_ui.ui \
		 ui/ndev_config_ui.ui \
		 ui/sae_register_ui.ui  \
		 ui/sae_info_ui.ui  \
		 ui/sae_status_ui.ui   \
		 ui/wireless_ui.ui \
		 ui/info_query_ui.ui \
		 ui/ticket_info_query_ui.ui \
		 ui/member_info_query_ui.ui \
		 ui/rent_info_query_ui.ui \
		 ui/device_info_ui.ui   \
		 ui/debug_tools_ui.ui \
		 ui/operation_guide_ui.ui \
		 ui/exception_handle_ui.ui \
		 ui/member_service_ui.ui \
		 ui/on_off_card_service_ui.ui \
    ui/recharge_service_ui.ui \
    ui/member_on_card_service_ui.ui \
    ui/delay_return_bicycle_ui.ui \
    ui/nearby_site_info_query_ui.ui
		 

		 







OBJECTS_DIR = ./debug/
