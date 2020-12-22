unit xtea;

interface

uses Classes;

{$OVERFLOWCHECKS OFF}

type TxteaKey = array[0..3] of LongWord;
type TxteaData = array[0..1] of LongWord;

const
  MIN_NUM_OF_ROUNDS: Byte = 16;
  MAX_NUM_OF_ROUNDS: Byte = 64;
  XTEA_KEY_LEN: Byte = 16;
  XTEA_PACKET_LEN: Byte = 8;

type
  Txtea = class(TPersistent)
  private
    FNumOfRounds: Byte;
    FKey: TxteaKey;
    procedure SetNumOfRows (Rounds: Byte);
    procedure SetKey(Key: string);
  public
    procedure CryptBlock(var Data : TxteaData);
    procedure DecryptBlock(var Data : TxteaData);
    procedure Crypt(var Data : Array of Byte; Len: Word);
    procedure Decrypt(var Data : Array of Byte; Len: Word);
    property NumOfRounds: Byte read FNumOfRounds write SetNumOfRows;
    property Key: string write SetKey;
  end;

implementation

function rol(var Base: LongWord; Shift: LongWord): LongWord;
  var res: LongWord;
begin
  // only 5 bits of shift are significant
  Shift := Shift and $1F;
  res := (Base shl Shift) or (Base shr (32 - Shift));
  result := res;
end;

procedure Txtea.CryptBlock(var Data : TxteaData);
var
  sum : LongWord;
  delta : LongWord;
  y, z: LongWord;
  n : Byte;
begin
  sum := 0;
  delta := $9E3779B9;
  // load and pre-white the registers
  y := Data[0] + FKey[0];
  z := Data[1] + FKey[1];
  // round functions
  for n := 1 to FNumOfRounds do
  begin
    y := y + ((z shl 4) xor (z shr 5)) + (z xor sum) + rol(FKey[sum and 3], z);
    sum := sum + delta;
    z := z + ((y shl 4) xor (y shr 5)) + (y xor sum) + rol(FKey[(sum shr 11) and 3], y);
  end;
  // post-white and store registers
  Data[0] := y xor FKey[2];
  Data[1] := z xor FKey[3];
end;

procedure Txtea.DecryptBlock(var Data : TxteaData);
var
  sum : LongWord;
  delta : LongWord;
  y, z: LongWord;
  n : Byte;
begin
  delta := $9E3779B9;
  sum := (delta * FNumOfRounds);// and $FFFFFFFF;

  z := Data[1] xor FKey[3];
  y := Data[0] xor FKey[2];

  for n := 1 to FNumOfRounds do
  begin
    z := z - (((y shl 4) xor (y shr 5)) + (y xor sum) + rol(FKey[(sum shr 11) and 3], y));
    sum := sum - delta;
    y := y - (((z shl 4) xor (z shr 5)) + (z xor sum) + rol(FKey[sum and 3], z));
  end;

  Data[1] := z - FKey[1];
  Data[0] := y - FKey[0];
end;

procedure Txtea.Crypt(var Data : Array of Byte; Len: Word);
var
  jx: Word;
  XteaData: TxteaData;
begin
  XteaData := Default(TxteaData);
  jx := 0;
  while jx < Len do
  begin
    FKey[3] := (FKey[3] and $00FFFFFF) or (LongWord(Chr(jx div XTEA_PACKET_LEN)) shl 24);
    Move(Data[jx], XteaData, XTEA_PACKET_LEN);
    CryptBlock(XteaData);
    Move(XteaData, Data[jx], XTEA_PACKET_LEN);
    jx := jx + XTEA_PACKET_LEN;
  end;
end;

procedure Txtea.Decrypt(var Data : Array of Byte; Len: Word);
var
  jx: Word;
  XteaData: TxteaData;
begin
  XteaData := Default(TxteaData);
  jx := 0;
  while jx < Len do
  begin
    FKey[3] := (FKey[3] and $00FFFFFF) or (LongWord(Chr(jx div XTEA_PACKET_LEN)) shl 24);
    Move(Data[jx], XteaData, XTEA_PACKET_LEN);
    DecryptBlock(XteaData);
    Move(XteaData, Data[jx], XTEA_PACKET_LEN);
    jx := jx + XTEA_PACKET_LEN;
  end;
end;

procedure Txtea.SetNumOfRows(Rounds: Byte);
begin
  if Rounds < MIN_NUM_OF_ROUNDS then Rounds := MIN_NUM_OF_ROUNDS;
  if Rounds > MAX_NUM_OF_ROUNDS then Rounds := MAX_NUM_OF_ROUNDS;
  FNumOfRounds := Rounds;
end;

procedure Txtea.SetKey(Key: string);
var
  Temp: TxteaKey;
  i: Integer;
begin
  for i := 0 to 3 do
    Temp[i] := 0;
  if Length(Key) > XTEA_KEY_LEN then
    Move(Key[1], Temp, XTEA_KEY_LEN)
  else
    Move(Key[1], Temp, Length(Key));
  FKey := Temp;
end;

end.
