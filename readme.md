#to install the solve 
apt-get install minisat 

#to compile the files 
make all 

#to execute piping with the view process. 
./master <FILENAMES> | ./view 

#they can be executed apart 
./master <FILENAMES> 
#this command will print two strings 
#then to print the result 
./view string1 string2

