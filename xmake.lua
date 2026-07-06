add_rules("mode.debug", "mode.release")

if is_mode("release") then
    set_policy("build.optimization.lto", true)
end

add_requires("cli11")
add_requires("fmt")
add_requires("glaze")
add_requires("ctre")
add_requires("fast_io")
add_requires("flux")

target("horos")
    set_languages("cxx23")
    add_rules("qt.quickapp")
    add_frameworks("QtQuickControls2", "QtQuick", "QtGui", "QtQml", "QtCore")

    add_headerfiles("src/*.hpp")
    add_files("src/*.cpp")
    add_files("src/*.hpp")

    add_files("qml.qrc")

    add_packages("glaze", "ctre", "fast_io", "flux", "fmt", "cli11")

    add_installfiles("assets/horos.desktop", {prefixdir = "share/applications"})
    add_installfiles("assets/horos.svg", {prefixdir = "share/icons/hicolor/scalable/apps"})
