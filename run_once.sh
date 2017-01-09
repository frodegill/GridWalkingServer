#!/bin/sh

sudo apt-get install g++ make mysql-server libmysqlclient-dev unixodbc unixodbc-dev
echo "Depends on Poco 1.6.1+. Download, compile and install the Complete Edition from http://pocoproject.org/"
echo "Depends on Restbed. Download, compile and install from https://github.com/corvusoft/restbed#build"

#Create database
echo "When asked for password, use sudo if applicable, then MySQL admin password"
sudo mysql -u root -p < ./prepare_gridwalking.sql
echo "When asked for password, use MySQL gridwalking password"
sudo mysql -u gridwalking -p gridwalking < ./create_gridwalking.sql

#Set up unixodbc (system) driver and (user) datasource
sudo odbcinst -i -l -d -f /usr/share/libmyodbc/odbcinst.ini
odbcinst -i -h -s -f ./datasource.template

make && make install
