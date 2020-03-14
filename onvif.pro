TEMPLATE = subdirs
SUBDIRS = \
    core \
    app

app.depends = core
