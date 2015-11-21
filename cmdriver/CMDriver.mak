# Microsoft Developer Studio Generated NMAKE File, Based on CMDriver.dsp
ifeq "$(CFG)" ""
CFG=CMDriver - Win32 Debug
$(warning No configuration specified. Defaulting to CMDriver - Win32 Debug.)
endif 

ifneq "$(CFG)" "CMDriver - Win32 Release" 
ifneq "$(CFG)" "CMDriver - Win32 Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "CMDriver.mak" CFG="CMDriver - Win32 Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "CMDriver - Win32 Release" (based on "Win32 (x86) Console Application"))
$(warning "CMDriver - Win32 Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe

ifeq  "$(CFG)" "CMDriver - Win32 Release"

OUTDIR=Release
INTDIR=Release/Obj

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ./CmXml/include /I $(PATH2BS)/thirdparty/xml4c/src \
	/I ../DevXLauncher/include /I ../nameserver/include /I $(PATH2BS)/DevXLauncher/include /I $(PATH2BS)/nameserver/include /D "WIN32" /D "NDEBUG" \
	/D "_CONSOLE" /D "_MBCS" /Fp$(INTDIR)/CMDriver.pch /YX /Fo"$(INTDIR)/" /c 
	
LINK32_FLAGS=advapi32.lib wsock32.lib xerces-c_1.lib /nologo /subsystem:console \
	/incremental:no /pdb:$(OUTDIR)/CMDriver.pdb /machine:I386 /out:$(OUTDIR)/CMDriver.exe \
	/libpath:$(PATH2BS)/thirdparty/xerces/NT 

else #  "$(CFG)" == "CMDriver - Win32 Debug"

OUTDIR=Debug
INTDIR=Debug/Obj

CPP_PROJ=/nologo /MDd /W3 /GX /ZI /Od /I ./CmXml/include \
		/I $(PATH2BS)/thirdparty/xml4c/src /I ../DevXLauncher/include /I $(PATH2BS)/DevXLauncher/include \
		/I ../nameserver/include /I $(PATH2BS)/nameserver/include /D "WIN32" /D "_DEBUG" /D "_CONSOLE" \
		/D "_MBCS" /FR$(INTDIR)/ /Fp$(INTDIR)/CMDriver.pch /YX /Fo"$(INTDIR)/" /GZ /c 

LINK32_FLAGS=advapi32.lib wsock32.lib xerces-c_1D.lib /nologo /subsystem:console /incremental:yes \
	/pdb:$(OUTDIR)/CMDriver.pdb /debug /machine:I386 /out:$(OUTDIR)/CMDriver.exe \
	/pdbtype:sept /libpath:$(PATH2BS)/thirdparty/xerces/NT /NODEFAULTLIB:msvcrt

endif 

ALL : $(OUTDIR)/CMDriver.exe 


CLEAN :
	-@rm $(INTDIR)/CMDriver.obj
	-@rm $(INTDIR)/CMDriver.sbr
	-@rm $(INTDIR)/CmXml.obj
	-@rm $(INTDIR)/CmXml.sbr
	-@rm $(INTDIR)/CmXmlCollection.obj
	-@rm $(INTDIR)/CmXmlCollection.sbr
	-@rm $(INTDIR)/CmXmlErrorHandler.obj
	-@rm $(INTDIR)/CmXmlErrorHandler.sbr
	-@rm $(INTDIR)/CmXmlException.obj
	-@rm $(INTDIR)/CmXmlException.sbr
	-@rm $(INTDIR)/CmXmlNode.obj
	-@rm $(INTDIR)/CmXmlNode.sbr
	-@rm $(INTDIR)/CmXmlStringTokenizer.obj
	-@rm $(INTDIR)/CmXmlStringTokenizer.sbr
	-@rm $(INTDIR)/CmXmlSystem.obj
	-@rm $(INTDIR)/CmXmlSystem.sbr
	-@rm $(INTDIR)/vc60.idb
	-@rm $(INTDIR)/vc60.pdb
	-@rm $(OUTDIR)/CMDriver.bsc
	-@rm $(OUTDIR)/CMDriver.exe
	-@rm $(OUTDIR)/CMDriver.ilk
	-@rm $(OUTDIR)/CMDriver.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

LINK32_OBJS= \
	$(INTDIR)/CMDriver.obj \
	$(INTDIR)/CmXml.obj \
	$(INTDIR)/CmXmlCollection.obj \
	$(INTDIR)/CmXmlErrorHandler.obj \
	$(INTDIR)/CmXmlException.obj \
	$(INTDIR)/CmXmlNode.obj \
	$(INTDIR)/CmXmlStringTokenizer.obj \
	$(INTDIR)/CmXmlSystem.obj \
	./libs/nameserver/$(OUTDIR)/nameserver.lib \
	./libs/DevXLauncher/$(OUTDIR)/DevXLauncher.lib	

$(OUTDIR)/CMDriver.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

$(INTDIR)/CmXml.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./CmXml/src/CmXml.cpp

$(INTDIR)/CmXmlCollection.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./CmXml/src/CmXmlCollection.cpp

$(INTDIR)/CmXmlErrorHandler.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./CmXml/src/CmXmlErrorHandler.cpp

$(INTDIR)/CmXmlException.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./CmXml/src/CmXmlException.cpp

$(INTDIR)/CmXmlNode.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./CmXml/src/CmXmlNode.cpp

$(INTDIR)/CmXmlStringTokenizer.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./CmXml/src/CmXmlStringTokenizer.cpp

$(INTDIR)/CmXmlSystem.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./CmXml/src/CmXmlSystem.cpp

$(INTDIR)/CMDriver.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/CMDriver.cxx

