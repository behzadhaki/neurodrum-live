[Setup]
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
AppName=NeuralDrumLiveV2
AppVersion=0.0.1
DefaultDirName={cf}
DefaultGroupName=NeuralDrumLiveV2
OutputBaseFilename=NeuralDrumLiveV2-win-x64

[Files]
Source: "C:/Program Files/Common Files/VST3/NeuralDrumLiveV2.vst3/*.*"; DestDir: "C:/Program Files/Common Files/VST3"; Flags: recursesubdirs createallsubdirs