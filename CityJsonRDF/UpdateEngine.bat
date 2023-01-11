
echo Check Engine includes
if .%RDF_ENGINE_INCLUDE%.==.. goto IncUpdated
echo echo Update Engine includes 
xcopy %RDF_ENGINE_INCLUDE% engine\include /F /Y
:IncUpdated

echo Check IFC engine binaries
if .%RDF_ENGINE_LIB%.==.. goto LibUpdated
echo Update lib and dll
xcopy %RDF_ENGINE_LIB%Debug\engine.lib engine\lib\%3\ /F /Y
xcopy %RDF_ENGINE_LIB%%2\engine.dll engine\lib\%3\%2\ /F /Y
echo Copy dll to output
xcopy engine\lib\%3\%2\engine.dll %1 /F /Y
:LibUpdated
