TEMPLATE = subdirs
SUBDIRS = openCtree commonfunction openRD
commonfunction.depends += openCtree
openRD.depends += commonfunction
