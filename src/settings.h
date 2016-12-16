#ifndef SETTINGS_H
#define SETTINGS_H

#define IMPERIAL_UNITS() \
	(QLocale::system().measurementSystem() == QLocale::ImperialSystem)

#define WINDOW_SETTINGS_GROUP             "Window"
#define WINDOW_SIZE_SETTING               "size"
#define WINDOW_SIZE_DEFAULT               QSize(600, 800)
#define WINDOW_POS_SETTING                "pos"
#define WINDOW_POS_DEFAULT                QPoint(10, 10)

#define SETTINGS_SETTINGS_GROUP           "Settings"
#define UNITS_SETTING                     "units"
#define UNITS_DEFAULT                     (IMPERIAL_UNITS() ? Imperial : Metric)
#define SHOW_TOOLBARS_SETTING             "toolbar"
#define SHOW_TOOLBARS_DEFAULT             true

#define GRAPH_SETTINGS_GROUP              "Graph"
#define SHOW_GRAPHS_SETTING               "show"
#define SHOW_GRAPHS_DEFAULT               true
#define GRAPH_TYPE_SETTING                "type"
#define GRAPH_TYPE_DEFAULT                Distance
#define SHOW_GRAPH_GRIDS_SETTING          "grid"
#define SHOW_GRAPH_GRIDS_DEFAULT          true

#define MAP_SETTINGS_GROUP                "Map"
#define CURRENT_MAP_SETTING               "map"
#define SHOW_MAP_SETTING                  "show"
#define SHOW_MAP_DEFAULT                  true

#define POI_SETTINGS_GROUP                "POI"
#define OVERLAP_POI_SETTING               "overlap"
#define OVERLAP_POI_DEFAULT               false
#define LABELS_POI_SETTING                "labels"
#define LABELS_POI_DEFAULT                true
#define SHOW_POI_SETTING                  "show"
#define SHOW_POI_DEFAULT                  false
#define DISABLED_POI_FILE_SETTINGS_PREFIX "disabled"
#define DISABLED_POI_FILE_SETTING         "file"

#define DATA_SETTINGS_GROUP               "Data"
#define SHOW_TRACKS_SETTING               "tracks"
#define SHOW_TRACKS_DEFAULT               true
#define SHOW_ROUTES_SETTING               "routes"
#define SHOW_ROUTES_DEFAULT               true
#define SHOW_WAYPOINTS_SETTING            "waypoints"
#define SHOW_WAYPOINTS_DEFAULT            true
#define SHOW_ROUTE_WAYPOINTS_SETTING      "routeWaypoints"
#define SHOW_ROUTE_WAYPOINTS_DEFAULT      true
#define SHOW_WAYPOINT_LABELS_SETTING      "waypointLabels"
#define SHOW_WAYPOINT_LABELS_DEFAULT      true

#define EXPORT_SETTINGS_GROUP             "Export"
#define PAPER_ORIENTATION_SETTING         "orientation"
#define PAPER_ORIENTATION_DEFAULT         QPrinter::Portrait
#define PAPER_SIZE_SETTING                "size"
#define PAPER_SIZE_DEFAULT                (IMPERIAL_UNITS() ? QPrinter::Letter \
                                            : QPrinter::A4)
#define MARGIN_LEFT_SETTING               "marginLeft"
#define MARGIN_LEFT_DEFAULT               5 /* mm */
#define MARGIN_TOP_SETTING                "marginTop"
#define MARGIN_TOP_DEFAULT                5 /* mm */
#define MARGIN_RIGHT_SETTING              "marginRight"
#define MARGIN_RIGHT_DEFAULT              5 /* mm */
#define MARGIN_BOTTOM_SETTING             "marginBottom"
#define MARGIN_BOTTOM_DEFAULT             5 /* mm */
#define EXPORT_FILENAME_SETTING           "fileName"
#define EXPORT_FILENAME_DEFAULT           QString("%1/export.pdf"). \
                                            arg(QDir::currentPath())

#define OPTIONS_SETTINGS_GROUP            "Options"
#define PALETTE_COLOR_SETTING             "paletteColor"
#define PALETTE_COLOR_DEFAULT             QColor(Qt::blue)
#define PALETTE_SHIFT_SETTING             "paletteShift"
#define PALETTE_SHIFT_DEFAULT             0.62
#define TRACK_WIDTH_SETTING               "trackWidth"
#define TRACK_WIDTH_DEFAULT               3
#define ROUTE_WIDTH_SETTING               "routeWidth"
#define ROUTE_WIDTH_DEFAULT               3
#define TRACK_STYLE_SETTING               "trackStyle"
#define TRACK_STYLE_DEFAULT               Qt::SolidLine
#define ROUTE_STYLE_SETTING               "routeStyle"
#define ROUTE_STYLE_DEFAULT               Qt::DotLine
#define GRAPH_WIDTH_SETTING               "graphWidth"
#define GRAPH_WIDTH_DEFAULT               1
#define PATH_AA_SETTING                   "pathAntiAliasing"
#define PATH_AA_DEFAULT                   true
#define GRAPH_AA_SETTING                  "graphAntiAliasing"
#define GRAPH_AA_DEFAULT                  false
#define POI_RADIUS_SETTING                "poiRadius"
#define POI_RADIUS_DEFAULT                (IMPERIAL_UNITS() ? MIINM : KMINM)
#define USE_OPENGL_SETTING                "useOpenGL"
#define USE_OPENGL_DEFAULT                false
#define PRINT_NAME_SETTING                "printName"
#define PRINT_NAME_DEFAULT                true
#define PRINT_DATE_SETTING                "printDate"
#define PRINT_DATE_DEFAULT                true
#define PRINT_DISTANCE_SETTING            "printDistance"
#define PRINT_DISTANCE_DEFAULT            true
#define PRINT_TIME_SETTING                "printTime"
#define PRINT_TIME_DEFAULT                true
#define PRINT_ITEM_COUNT_SETTING          "printItemCount"
#define PRINT_ITEM_COUNT_DEFAULT          true
#define SEPARATE_GRAPH_PAGE_SETTING       "separateGraphPage"
#define SEPARATE_GRAPH_PAGE_DEFAULT       false

#endif // SETTINGS_H
