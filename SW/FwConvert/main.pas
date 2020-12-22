unit main;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, ComCtrls,
  ExtCtrls, StdCtrls, EditBtn, vinfo, calccrc, XMLConf, xtea;

type

  { TmainForm }

  TmainForm = class(TForm)
    btnConvert: TButton;
    cbBaud1: TComboBox;
    cbBaud2: TComboBox;
    cbDevices: TComboBox;
    cbMajVer: TComboBox;
    cbMinVer: TComboBox;
    edFlash: TFileNameEdit;
    edResult: TFileNameEdit;
    lblVerPoint: TLabel;
    lblVersion: TLabel;
    lblFlash: TLabel;
    lblBaud1: TLabel;
    lblBaud2: TLabel;
    lblDevice: TLabel;
    lblResult: TLabel;
    pnMain: TPanel;
    StatusBar1: TStatusBar;
    XmlConfig: TXMLConfig;
    procedure edFlashButtonClick(Sender: TObject);
    procedure edResultButtonClick(Sender: TObject);
    procedure SaveSettings;
    procedure btnConvertClick(Sender: TObject);
    procedure cbBaud1Change(Sender: TObject);
    procedure edFlashChange(Sender: TObject);
    procedure edResultChange(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    { private declarations }
  public
    { public declarations }
  end;

const
     IDENT_SIZE: Integer = 2;

     PAGE_SIZE: Word = 256;

     HEX_DATA: Char = chr($00);
     HEX_END: Char = chr($01);
     HEX_SEG: Char = chr($02);
     HEX_SSTART: Char = chr($03);
     HEX_ADDADDR: Char = chr($04);

     XTEA_PACKET_LEN: Integer = 8;

     XTEA_KEY: string = 'FwUpdate25121978';

     STATUS_BAR_SIZE: Byte = 0;
     STATUS_BAR_PAGES: Byte = 1;
     STATUS_BAR_INFO: Byte = 2;

type DEVICES =
  (
    DEVICE_LS1,                //0
    DEVICE_LAST
  );

type TheadFile = Record
    Id: Array[0..1] of Byte;
    start: Word;
    majVer: Byte;
    minVer: Byte;
    device: Word;
    numPages: Word;
    baud1: Byte;
    baud2: Byte;
    date: LongWord;
    reserved: Array[0..15] of Byte;    // 16 bytes for future usage
end;

const cmbBaud: Array[0..10] of PChar =
(
    '1200',
    '2400',
    '4800',
    '9600',
    '14400',
    '19200',
    '38400',
    '56000',
    '57600',
    '115200',
    NIL
);

const cmbDevices: Array[0..1] of PChar =
(
    'LS1',                   //0
    NIL
);

const FILE_IDENT: PChar = 'FW';

var
  mainForm: TmainForm;
  InDirectory: string;
  OutDirectory: string;

implementation

function toint_new(datab: Char): Byte;
var
   input: Byte;
begin
   input := 0;

   if ((Byte(datab) >= $30) and (Byte(datab) <= $39)) then
     input := Byte(datab) - $30
   else
   if ((Byte(datab) >= $41) and (Byte(datab) <= $46)) then
     input := Byte(datab) - $37
   else
   if ((Byte(datab) >= $61) and (Byte(datab) <= $66)) then
     input := Byte(datab) - $57;
   Result := input;
end;

function hex2char(str: PChar): Byte;
var
   retval: Byte;
   byteH, byteL: Char;
begin

   // get a two-byte ASCII representation of a char from the UART
   byteH := str[0];
   Inc(str);
   byteL := str[0];
   Inc(str);

   // convert to a single 8 bit result
   retval := toint_new(byteH) * 16 + toint_new(byteL);
   Result := retval;
end;

{$R *.lfm}

{ TmainForm }

procedure TmainForm.SaveSettings;
begin
  XmlConfig.Clear;
  XmlConfig.OpenKey('Main');
  XmlConfig.SetValue('InDirectory', WideString(InDirectory));
  XmlConfig.SetValue('OutDirectory', WideString(OutDirectory));
  XmlConfig.CloseKey;
  XmlConfig.Flush;
end;

procedure TmainForm.edFlashButtonClick(Sender: TObject);
begin
  edFlash.InitialDir := InDirectory;
end;

procedure TmainForm.edResultButtonClick(Sender: TObject);
begin
  edResult.InitialDir := OutDirectory;
end;

procedure TmainForm.btnConvertClick(Sender: TObject);
var
  f_src: TextFile;
  len: Byte;                     // holds the HEX record length field
  record_type: Byte;             // holds the HEX record type field
  offset: Longword;              // holds the HEX record offset field
  first_addr: Longword = $FFFFFFFF;
  CRC32: Longword;
  CRC16: Word;
  bwrite: Byte;
  msg: string;
  headFile: TheadFile;
  ix, jx: Integer;
  add_segment: Longword;
  curPos: Integer;
  outStream: TFileStream;
  headOffs: Longword;
  TS: TTimeStamp;
  Counter: Integer = 0;
  buffer: ^Byte;
  xtea: Txtea;
  buff: Array[0..255] of Byte;

begin
  headFile := Default(TheadFile);
  buff[0] := 0;

  // start convertion
  if (edFlash.FileName = '') then
  begin
    ShowMessage('Please select HEX-file with flash data first!');
    Exit;
  end;
  if (not(FileExists(edFlash.FileName))) then
  begin
    ShowMessage('Cann''t open HEX-file ' + edFlash.FileName + '!');
    Exit;
  end;

  if (edResult.FileName = '') then
  begin
    ShowMessage('Please enter the name of the result file first!');
    Exit;
  end;

  add_segment := 0;

  try
    AssignFile(f_src, edFlash.FileName);
    Reset(f_src);
  except
    ShowMessage('Cann''t open HEX-file ' + edFlash.FileName + '!');
    Exit;
  end;

  outStream := TFileStream.Create(edResult.FileName, fmCreate);

  Move(FILE_IDENT^, headFile.Id, IDENT_SIZE);
  headFile.start := SizeOf(TheadFile);
  headFile.device := cbDevices.ItemIndex;
  headFile.baud1 := Byte(cbBaud1.ItemIndex);
  headFile.baud2 := Byte(cbBaud2.ItemIndex);
  headFile.majVer := cbMajVer.ItemIndex;
  headFile.minVer := cbMinVer.ItemIndex;
  headFile.numPages := 0;
  TS := DateTimeToTimeStamp(Now);
  headFile.date := TS.Date;

  headOffs := SizeOf(TheadFile);

  outStream.Seek(headOffs, soFromBeginning);

  StatusBar1.Panels.Items[STATUS_BAR_INFO].Text := 'Converting...';
  Application.ProcessMessages;

  while not Eof(f_src) do
  begin
    readln(f_src, msg);
    ix := 1;
    while ((msg[ix] <> ':') and (msg[ix] <> chr(0))) do Inc(ix);
    if (msg[ix] = chr(0)) then continue;
    Inc(ix);

    // get the record length
    len := hex2char(@msg[ix]);
    ix := ix + 2;

    // get the starting address (offset field in HEX record)
    offset := hex2char(@msg[ix]);               // get the MSB
    ix := ix + 2;
    offset := offset << 8;
    offset := offset + hex2char(@msg[ix]);      // get the LSB
    ix := ix + 2;

    // get the record type
    record_type := hex2char(@msg[ix]);
    ix := ix + 2;

    if (record_type = Ord(HEX_SEG)) then
    begin
      //get segment offset
      offset := hex2char(@msg[ix]);              // get the MSB
      ix := ix + 2;
      offset := offset << 8;
      offset := offset + hex2char(@msg[ix]);     // get the LSB
      ix := ix + 2;
      add_segment := offset << 4;
      continue;
    end;

    if (record_type = Ord(HEX_ADDADDR)) then
    begin
      //get additional address
      offset := hex2char(@msg[ix]);             // get the MSB
      ix := ix + 2;
      offset := offset << 8;
      offset := offset + hex2char(@msg[ix]);    // get the LSB
      ix := ix + 2;
      add_segment := offset << 16;
    end;

    if (record_type <> Ord(HEX_DATA)) then
      continue;

    offset := offset + add_segment;

    if (offset < first_addr) then
    begin
       first_addr := offset;
    end;

    offset := offset + headOffs - first_addr;
    outStream.Seek(offset, soFromBeginning);

    while (ix < (len * 2 + 9)) do
    begin
       bwrite := hex2char(@msg[ix]);   // write one byte to file
       outStream.Write(bwrite, SizeOf(Byte));
       Inc(Counter);
       ix := ix + 2;
    end;

  end;

  CloseFile(f_src);

  StatusBar1.Panels.Items[STATUS_BAR_SIZE].Text := 'Size: ' + IntToStr(Counter);

  bwrite := $FF;
  while (Counter mod PAGE_SIZE) <> 0 do
  begin
    outStream.Write(bwrite, SizeOf(Byte));
    Inc(Counter);
  end;

  headFile.numPages := Counter div PAGE_SIZE;

  StatusBar1.Panels.Items[STATUS_BAR_PAGES].Text := 'Pages: ' + IntToStr(headFile.numPages);

  curPos := outStream.Position;

  //calculate CRC16 checksums for every page
  outStream.Seek(headOffs, soFromBeginning);
  buffer := GetMem(headFile.numPages * SizeOf(Word));
  for ix := 0 to (headFile.numPages - 1) do
  begin
    CRC16 := $FFFF;
    for jx := 0 to (PAGE_SIZE - 1) do
    begin
      outStream.Read(bwrite, SizeOf(Byte));
      CRC16 := CalcCRC16(CRC16, bwrite);
    end;
    buffer[ix * SizeOf(Word)] := CRC16 mod $100;
    buffer[ix * SizeOf(Word) + 1] := CRC16 div $100;
    //Move(CRC16, buffer[ix * SizeOf(Word)], SizeOf(Word));
  end;
  outStream.Seek(curPos, soFromBeginning);
  outStream.Write(buffer^, headFile.numPages * SizeOf(Word));
  FreeMem(buffer);

  curPos := outStream.Position;

  outStream.Seek(0, soFromBeginning);
  outStream.Write(&headFile, SizeOf(headFile));

  //encode data
  outStream.Seek(headOffs, soFromBeginning);
  xtea := Txtea.Create;
  xtea.NumOfRounds := 32;
  xtea.Key := XTEA_KEY;
  for ix := 0 to (headFile.numPages - 1) do
  begin
    outStream.Read(buff, PAGE_SIZE);
    xtea.Crypt(buff, PAGE_SIZE);
    outStream.Seek(-PAGE_SIZE, soFromCurrent);
    outStream.Write(buff, PAGE_SIZE);
    {jx := 0;
    while jx < PAGE_SIZE do
    begin
      key := XTEA_KEY;
      key[16] := chr(jx div XTEA_PACKET_LEN);
      xtea.Key := key;
      outStream.Read(Data, XTEA_PACKET_LEN);
      xtea.CryptBlock(Data);
      outStream.Seek(-XTEA_PACKET_LEN, soFromCurrent);
      outStream.Write(Data, XTEA_PACKET_LEN);
      jx := jx + XTEA_PACKET_LEN;
    end;}
  end;
  xtea.Free;

  outStream.Seek(0, soFromBeginning);
  CRC32 := $FFFFFFFF;
  for ix := 0 to (curPos - 1) do
  begin
    outStream.Read(bwrite, SizeOf(Byte));
    CRC32 := CalcCRC32(CRC32, bwrite);
  end;

  outStream.Write(CRC32, SizeOf(Longword));
  curPos := curPos + 4;

  outStream.Free;

  StatusBar1.Panels.Items[STATUS_BAR_INFO].Text := 'Ready!';
end;

procedure TmainForm.cbBaud1Change(Sender: TObject);
begin
  cbBaud2.ItemIndex := cbBaud1.ItemIndex;
end;

procedure TmainForm.edFlashChange(Sender: TObject);
begin
  InDirectory := ExtractFilePath(edFlash.FileName);
  SaveSettings;
end;

procedure TmainForm.edResultChange(Sender: TObject);
begin
  OutDirectory := ExtractFilePath(edResult.FileName);
  SaveSettings;
end;

procedure TmainForm.FormCreate(Sender: TObject);
Var
  MajorNum : String;
  MinorNum : String;
  RevisionNum : String;
  BuildNum : String;
  Info: TVersionInfo;
  MyVersion: String;
  ix: Integer;
begin
  XmlConfig.Filename := ChangeFileExt(Paramstr(0), '.xml');
  XmlConfig.OpenKey('Main');
  InDirectory := AnsiString(XmlConfig.GetValue('InDirectory', ''));
  OutDirectory := AnsiString(XmlConfig.GetValue('OutDirectory', ''));
  XmlConfig.CloseKey;
  // check software version
  Info := TVersionInfo.Create;
  Info.Load(HINSTANCE);
  MajorNum := IntToStr(Info.FixedInfo.FileVersion[0]);
  MinorNum := IntToStr(Info.FixedInfo.FileVersion[1]);
  RevisionNum := IntToStr(Info.FixedInfo.FileVersion[2]);
  BuildNum := IntToStr(Info.FixedInfo.FileVersion[3]);
  Info.Free;

  MyVersion := MajorNum+'.'+MinorNum+'.'+RevisionNum+'.'+BuildNum;
  mainForm.Caption := 'Firmware Generator ver.' + MyVersion;

  ix := 0;
  while (cmbBaud[ix] <> NIL) do
  begin
    cbBaud1.Items.Add(cmbBaud[ix]);
    cbBaud2.Items.Add(cmbBaud[ix]);
    Inc(ix);
  end;
  cbBaud1.ItemIndex := 0;
  cbBaud2.ItemIndex := 0;

  ix := 0;
  while (cmbDevices[ix] <> NIL) do
  begin
    cbDevices.Items.Add(cmbDevices[ix]);
    Inc(ix);
  end;
  cbDevices.ItemIndex := 0;

  for ix := 0 to 99 do
  begin
    cbMajVer.Items.Add(IntToStr(ix));
    cbMinVer.Items.Add(IntToStr(ix));
  end;
  cbMajVer.ItemIndex := 0;
  cbMinVer.ItemIndex := 0;
end;

end.

