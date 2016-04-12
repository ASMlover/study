cd ..
if not exist PyProto mkdir PyProto
cd Proto

protoc.exe --python_out=../PyProto Common.proto ClientGate.proto GateGame.proto DBManager.proto GameManager.proto

pause
