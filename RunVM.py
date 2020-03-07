#  Python to run Assembler for all asm files
import os
import filecmp
# file = "Add.asm"
# os.system("Assembler.exe "+ file)
# fn,ext=file.split(".")
# fn +=".hack2"
# print(fn)
# if(filecmp.cmp(fn+".hack",fn+".hack2",shallow=False)==False):
	# print("failed for",file,"..exiting")
	# exit()
# print("exe")
dirpath = os.getcwd()
print("current directory is : " + dirpath)
foldername = os.path.basename(dirpath)
print("Directory name is : " + foldername)
for root, dirs, files in os.walk(os.curdir):
    for file in files:
        if file.endswith(".vm"):
            os.system("VmTranslator.exe "+ file+" "+foldername)
print("Success")