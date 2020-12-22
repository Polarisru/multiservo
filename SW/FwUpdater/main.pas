unit main;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, ComCtrls,
  ExtCtrls, StdCtrls, EditBtn, Buttons, vinfo, Registry, synaser, calccrc;

type

  { TmainForm }

  TmainForm = class(TForm)
    btnReset: TButton;
    btnRefresh: TBitBtn;
    btnWrite: TButton;
    btnCheck: TButton;
    cbSerialPort: TComboBox;
    cbConnect: TCheckBox;
    editFirmwareFile: TFileNameEdit;
    gbFirmwareFile: TGroupBox;
    gbSerialPort: TGroupBox;
    lblDate: TLabel;
    lblSize: TLabel;
    lblVersion: TLabel;
    pnlConnect: TPanel;
    pnlMain: TPanel;
    ProgressBar1: TProgressBar;
    StatusBar1: TStatusBar;
    Timer1: TTimer;
    procedure btnCheckClick(Sender: TObject);
    procedure btnRefreshClick(Sender: TObject);
    procedure btnResetClick(Sender: TObject);
    procedure cbConnectChange(Sender: TObject);
    procedure editFirmwareFileChange(Sender: TObject);
    procedure ActivateButtons(On: boolean);
    procedure StopUpdate;
    procedure btnWriteClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure StatusBar1DrawPanel(StatusBar: TStatusBar; Panel: TStatusPanel;
      const Rect: TRect);
    procedure Timer1Timer(Sender: TObject);
  private
    { private declarations }
  public
    { public declarations }
  end;

const
  IDENT_SIZE: Integer = 2;

  GET_RES_OK: Byte    = 0;
  GET_RES_NOTOK: Byte = 1;
  GET_RES_ERR: Byte   = 2;

  ANSW_ERROR: Char = 'E';
  ANSW_OK: Char    = 'A';
  ANSW_SYNC1: Char = 'S';
  ANSW_SYNC2: Char = 'Y';

  CONNECT_ERRORS: Integer =  3;

  PACKET_OFFS_HEAD: Byte = 0;
  PACKET_OFFS_CMD: Byte  = 1;
  PACKET_OFFS_PAGE: Byte = 2;
  PACKET_OFFS_DATA: Byte = 4;
  PACKET_OFFS_CRC: Byte  = 6;

  PACKET_SIZE: Byte = 8;

  PACKET_HEADER: Byte = $A5;

  CMD_FLASH_PAGE: Byte  = $81;
  CMD_CHECK_FLASH: Byte = $83;
  CMD_FILL_BUFF: Byte   = $86;
  CMD_RUN_APP: Byte     = $A0;
  CMD_SYNC: Byte        = $90;

  PAGE_SIZE: Word = 256;

type DEVICES =
(
  DEVICE_LS1,                 //0
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

const cmbBaud: Array[0..9] of Integer =
(
    1200,
    2400,
    4800,
    9600,
    14400,
    19200,
    38400,
    56000,
    57600,
    115200
);

const FILE_IDENT: PChar = 'FW';

const DEV_STR_UPDATE: AnsiString = #27'BL1234'#13;
const DEV_STR_OK: PChar = 'O'#13;

var
  mainForm: TmainForm;
  ComPort1: TBlockSerial;
  inStream: TFileStream;
  timerStep: Byte;
  fileLoaded: boolean;
  headFile: TheadFile;
  colorStatus: TColor = clDefault;

implementation

function GetSerialPortRegNames: string;
var
  reg: TRegistry;
  l, v: TStringList;
  n: integer;
begin
  l := TStringList.Create;
  v := TStringList.Create;
  reg := TRegistry.Create;
  try
{$IFNDEF VER100}
    reg.Access := KEY_READ;
{$ENDIF}
    reg.RootKey := HKEY_LOCAL_MACHINE;
    reg.OpenKeyReadOnly('HARDWARE\DEVICEMAP\SERIALCOMM');//, false);
    reg.GetValueNames(l);
    for n := 0 to l.Count - 1 do
        v.Add(reg.ReadString(l[n]));
    Result := v.CommaText;
  finally
    reg.Free;
    l.Free;
    v.Free;
  end;
end;

function GetSerialPortNamesExt: string;
var
  reg  : TRegistry;
  l,v  : TStringList;
  n    : integer;
  pn,fn: string;

  function findFriendlyName(key: string; port: string): string;
  var
    r : TRegistry;
    k : TStringList;
    i : Integer;
    ck: string;
    rs: string;
  begin
    r := TRegistry.Create;
    k := TStringList.Create;

    r.RootKey := HKEY_LOCAL_MACHINE;
    r.OpenKeyReadOnly(key);
    r.GetKeyNames(k);
    r.CloseKey;

    try
      for i := 0 to k.Count - 1 do
      begin
        ck := key + k[i] + '\'; // current key
        // looking for "PortName" stringvalue in "Device Parameters" subkey
        if r.OpenKeyReadOnly(ck + 'Device Parameters') then
        begin
          if r.ReadString('PortName') = port then
          begin
            //Memo1.Lines.Add('--> ' + ck);
            r.CloseKey;
            r.OpenKeyReadOnly(ck);
            rs := r.ReadString('FriendlyName');
            Break;
          end // if r.ReadString('PortName') = port ...
        end  // if r.OpenKeyReadOnly(ck + 'Device Parameters') ...
        // keep looking on subkeys for "PortName"
        else // if not r.OpenKeyReadOnly(ck + 'Device Parameters') ...
        begin
          if r.OpenKeyReadOnly(ck) and r.HasSubKeys then
          begin
            rs := findFriendlyName(ck, port);
            if rs <> '' then Break;
          end; // if not (r.OpenKeyReadOnly(ck) and r.HasSubKeys) ...
        end; // if not r.OpenKeyReadOnly(ck + 'Device Parameters') ...
      end; // for i := 0 to k.Count - 1 ...
      result := rs;
    finally
      r.Free;
      k.Free;
    end; // try ...
  end; // function findFriendlyName ...

begin
  v      := TStringList.Create;
  l      := TStringList.Create;
  reg    := TRegistry.Create;
  Result := '';

  try
    reg.RootKey := HKEY_LOCAL_MACHINE;
    if reg.OpenKeyReadOnly('HARDWARE\DEVICEMAP\SERIALCOMM') then
    begin
      reg.GetValueNames(l);

      for n := 0 to l.Count - 1 do
      begin
        pn := reg.ReadString(l[n]);
        fn := findFriendlyName('\System\CurrentControlSet\Enum\', pn);
        v.Add(pn + ' = '+ fn);
      end; // for n := 0 to l.Count - 1 ...

      Result := v.CommaText;
    end; // if reg.OpenKeyReadOnly('HARDWARE\DEVICEMAP\SERIALCOMM') ...
  finally
    reg.Free;
    v.Free;
  end; // try ...
end;

procedure SendPacket(Cmd: Byte; Page: Word; Data: Word);
var
  packet: Array[0..7] of Byte;
  CRC16: Word;
  ix: Byte;
begin
  ComPort1.Purge;
  packet[PACKET_OFFS_HEAD] := PACKET_HEADER;
  packet[PACKET_OFFS_CMD] := Cmd;
  packet[PACKET_OFFS_PAGE] := Byte(Page div $100);
  packet[PACKET_OFFS_PAGE + 1] := Byte(Page mod $100);
  packet[PACKET_OFFS_DATA] := Byte(Data div $100);
  packet[PACKET_OFFS_DATA + 1] := Byte(Data mod $100);
  crc16 := $FFFF;
  for ix := 0 to (PACKET_SIZE - SizeOf(Word) - 1) do
  begin
    crc16 := CalcCRC16(crc16, packet[ix]);
  end;
  packet[PACKET_OFFS_CRC] := Byte(crc16 div $100);
  packet[PACKET_OFFS_CRC + 1] := Byte(crc16 mod $100);
  ComPort1.SendBuffer(@packet, PACKET_SIZE);
end;

function GetPacket(B1, B2: Byte; Timeout: Word): Byte;
var
  ix: Byte;
  in_buf: array[0..1] of Byte;
begin
  ix := ComPort1.RecvBufferEx(@in_buf, 2, Timeout);
  if (ix <> 2) then
    Result := GET_RES_ERR
  else if (in_buf[0] <> B1) or (in_buf[1] <> B2) then
    Result := GET_RES_NOTOK
  else
    Result := GET_RES_OK;
end;

{$R *.lfm}

{ TmainForm }

procedure TmainForm.FormCreate(Sender: TObject);
Var
  MajorNum : String;
  MinorNum : String;
  RevisionNum : String;
  BuildNum : String;
  Info: TVersionInfo;
  MyVersion: String;
begin
  // check software version
  Info := TVersionInfo.Create;
  Info.Load(HINSTANCE);
  MajorNum := IntToStr(Info.FixedInfo.FileVersion[0]);
  MinorNum := IntToStr(Info.FixedInfo.FileVersion[1]);
  RevisionNum := IntToStr(Info.FixedInfo.FileVersion[2]);
  BuildNum := IntToStr(Info.FixedInfo.FileVersion[3]);
  Info.Free;

  MyVersion := MajorNum + '.' + MinorNum + '.' + RevisionNum + '.' + BuildNum;
  mainForm.Caption := 'Firmware Updater ver.' + MyVersion;

  cbSerialPort.Items.CommaText := GetSerialPortRegNames;
  cbSerialPort.ItemIndex := 0;
  ProgressBar1.Parent := StatusBar1;
  fileLoaded := false;
end;

procedure TmainForm.StatusBar1DrawPanel(StatusBar: TStatusBar;
  Panel: TStatusPanel; const Rect: TRect);
var
  RectForText: TRect;
begin
  // draw status bar and progress bar on it
  if (Panel.Index = 1) then
  begin
    ProgressBar1.BoundsRect := Rect;
    ProgressBar1.PaintTo(StatusBar1.Canvas.Handle, Rect.Left, Rect.Top);
  end else
  if (Panel.Index = 0) then
  begin
    Canvas.Font.Color := colorStatus;
    RectForText := Rect;
    Canvas.FillRect(RectForText);
    StatusBar.Canvas.TextRect(Rect, Rect.Left + 5, Rect.Top, Panel.Text);
  end;
end;

procedure TmainForm.Timer1Timer(Sender: TObject);
var
  baudR: Integer;
begin
  if Timer1.Enabled then
  begin
    case timerStep of
    0:
      begin
        ComPort1 := TBlockSerial.Create;
        ComPort1.Connect(cbSerialPort.Items.Strings[cbSerialPort.ItemIndex]);
        if (ComPort1.LastError <> 0) then
        begin
          Timer1.Enabled := false;
          cbConnect.Checked := false;
          ComPort1.Free;
          ShowMessage('COM Port is busy! Please unplug it and try again!');
          Exit;
        end;
        baudR := cmbBaud[headFile.baud1];
        ComPort1.config(baudR, 8, 'N', SB1, False, False);
        ComPort1.SendString(DEV_STR_UPDATE);
        timerStep := 1;
      end;
    1:
      begin
        baudR := cmbBaud[headFile.baud2];
        ComPort1.config(baudR, 8, 'N', SB1, False, False);
        ComPort1.Purge;
        timerStep := 2;
      end;
    2:
      begin
        SendPacket(CMD_SYNC, $12AA, $3455);
        if GetPacket(Byte(ANSW_SYNC1), Byte(ANSW_SYNC2), 20) = GET_RES_OK then
        begin
          pnlConnect.Visible := true;
          if fileLoaded then
          begin
            cbConnect.Enabled := false;
            ActivateButtons(true);
          end;
        end else
        begin
          pnlConnect.Visible := false;
        end;
      end;
    3:
      begin

      end;
    end;
  end;
end;

procedure TmainForm.ActivateButtons(On: boolean);
begin
  if On then
  begin
    btnWrite.Enabled := true;
    btnCheck.Enabled := true;
    btnReset.Enabled := true;
  end else
  begin
    btnWrite.Enabled := false;
    btnCheck.Enabled := false;
    btnReset.Enabled := false;
  end;
end;

procedure TmainForm.StopUpdate;
begin
  {try
    ComPort1.Free;
  except

  end;
  try
    inStream.Free;
  finally
  end;}

  StatusBar1.Panels.Items[0].Text := '';
  ProgressBar1.Visible := false;
  ProgressBar1.Position := 0;
  //btnWrite.Enabled := true;
  btnRefresh.Enabled := true;
  editFirmwareFile.Enabled := true;
  cbSerialPort.Enabled := true;
  btnRefresh.Enabled := true;
  Timer1.Enabled := true;
end;

procedure TmainForm.btnRefreshClick(Sender: TObject);
begin
  // refresh COM-port list
  cbSerialPort.Enabled := false;
  cbSerialPort.Items.Clear;
  cbSerialPort.Items.CommaText := GetSerialPortRegNames;
  cbSerialPort.ItemIndex := 0;
  cbSerialPort.Enabled := true;
  if (cbSerialPort.Items.Count > 0) and fileLoaded then
    cbConnect.Enabled := true
  else
    cbConnect.Enabled := false;
end;

procedure TmainForm.btnResetClick(Sender: TObject);
begin
  Timer1.Enabled := false;
  Sleep(50);
  Application.ProcessMessages;
  SendPacket(CMD_RUN_APP, 0, 0);
  Sleep(10);
  cbConnect.Checked := false;
  cbConnect.Enabled := true;
  cbSerialPort.Enabled := true;
  editFirmwareFile.Enabled := true;
  btnRefresh.Enabled := true;
  ActivateButtons(false);
end;

procedure TmainForm.cbConnectChange(Sender: TObject);
begin
  if cbConnect.Checked then
  begin
    timerStep := 0;
    Timer1.Enabled := true;
  end else
  begin
    Timer1.Enabled := false;
    ComPort1.Free;
    pnlConnect.Visible := false;
  end;
end;

procedure TmainForm.editFirmwareFileChange(Sender: TObject);
var
  fSize: LongWord;
  counter: LongWord;
  CRC32, nCRC32: LongWord;
  ix: Byte;
  TS: TTimeStamp;
begin
  fileLoaded := false;

  headFile := Default(TheadFile);
  try
    inStream := TFileStream.Create(editFirmwareFile.FileName, fmOpenRead);
  except
    ShowMessage('Cann''t open firmware file ' + editFirmwareFile.FileName + '!');
    Exit;
  end;

  inStream.Seek(0, soFromBeginning);
  inStream.Read(&headFile, SizeOf(headFile));

  //check file identifier
  if (not(CompareMem(@headFile.Id, FILE_IDENT, IDENT_SIZE))) then
  begin
    ShowMessage('File seems not to be a right firmware file.'#13#10'File corrupted?');
    inStream.Free;
    Exit;
  end;
  //check file CRC
  CRC32 := $FFFFFFFF;
  fsize := inStream.Size;
  ix := 0;
  inStream.Seek(0, soFromBeginning);
  for counter := 0 to (fsize - SizeOf(Integer) - 1) do
  begin
    inStream.Read(ix, SizeOf(Byte));
    CRC32 := CalcCRC32(CRC32, ix);
  end;

  nCRC32 := 0;
  inStream.Read(nCRC32, SizeOf(Integer));

  if (CRC32 <> nCRC32) then
  begin
    ShowMessage('Checksum failed. File corrupted?');
    inStream.Free;
    Exit;
  end;

  // show info about firmware
  lblSize.Caption := Format('Size: %d', [LongWord(headFile.numPages) * PAGE_SIZE]);
  lblSize.Visible := true;
  lblVersion.Caption := Format('Version: %.2d.%.2d', [headFile.majVer, headFile.minVer]);
  lblVersion.Visible := true;
  TS.Time := 0;
  TS.Date := headFile.date;
  lblDate.Caption := Format('Date: %s',[DateToStr(TimeStampToDateTime(TS))]);
  lblDate.Visible := true;

  //inStream.Free;
  fileLoaded := true;
  if cbSerialPort.Items.Count > 0 then
    cbConnect.Enabled := true;
end;

procedure TmainForm.btnCheckClick(Sender: TObject);
var
  CRC16: Word;
  tError: Integer;
  numPage: Word;
  res: Byte;
begin
  Timer1.Enabled := false;
  ActivateButtons(false);
  btnRefresh.Enabled := false;
  editFirmwareFile.Enabled := false;
  cbSerialPort.Enabled := false;

  colorStatus := clDefault;

  ProgressBar1.Visible := true;
  ProgressBar1.Max := headFile.numPages;
  ProgressBar1.Position := 0;
  Application.ProcessMessages;

  numPage := 0;
  tError := 0;
  CRC16 := 0;
  res := GET_RES_OK;
  while (numPage < headFile.numPages) do
  begin
    if (tError > CONNECT_ERRORS) then
    begin
      StopUpdate;
      //ShowMessage('No response received.'#13#10'Check connection!');
      if (res = GET_RES_ERR) then
        StatusBar1.Panels.Items[0].Text := 'No connection'
      else
        StatusBar1.Panels.Items[0].Text := 'Wrong CRC. Page: ' + IntToStr(numPage + 1);
      colorStatus := clRed;
      StatusBar1.Repaint;
      Exit;
    end;
    StatusBar1.Panels.Items[0].Text := 'Checking page: ' + IntToStr(numPage + 1) +
                                    ' of ' + IntToStr(headFile.numPages);
    inStream.Seek(headFile.start + QWord(headFile.numPages) * PAGE_SIZE + numPage * SizeOf(Word), soFromBeginning);
    inStream.Read(CRC16, SizeOf(Word));
    SendPacket(CMD_CHECK_FLASH, numPage, CRC16);
    res := GetPacket(Byte(ANSW_OK), Byte(ANSW_OK), 50);
    if res <> GET_RES_OK then
    begin
      Inc(tError);
      continue;
    end;
    tError := 0;
    Inc(numPage);
    ProgressBar1.Position := numPage;
    Application.ProcessMessages;
  end;

  StatusBar1.Panels.Items[0].Text := 'Flash Ok!';
  ProgressBar1.Visible := false;
  ActivateButtons(true);
end;

procedure TmainForm.btnWriteClick(Sender: TObject);
var
  CRC16: Word;
  ix: Word;
  tError: Integer;
  out_buf: Array [0..257] of Byte;
  numPage: Word;
  res: Byte;
  buff_crc: ^Byte;
begin
  // start update procedure
  headFile := Default(TheadFile);
  out_buf[0] := 0;
  res := 0;

  ActivateButtons(false);

  inStream.Seek(0, soFromBeginning);
  inStream.Read(&headFile, SizeOf(headFile));

  btnWrite.Enabled := false;
  btnRefresh.Enabled := false;
  editFirmwareFile.Enabled := false;
  cbSerialPort.Enabled := false;

  ComPort1.Purge;

  StatusBar1.Panels.Items[0].Text := 'Start bootloader...';
  Application.ProcessMessages;

  ProgressBar1.Visible := true;
  ProgressBar1.Max := headFile.numPages;
  ProgressBar1.Position := 0;
  Application.ProcessMessages;

  // read CRCs from the end of file
  buff_crc := GetMem(headFile.numPages * SizeOf(Word));
  inStream.Seek(headFile.start + QWord(headFile.numPages) * PAGE_SIZE, soFromBeginning);
  inStream.Read(buff_crc^, headFile.numPages * SizeOf(Word));

  inStream.Seek(SizeOf(headFile), soFromBeginning);

  numPage := 0;
  tError := 0;
  while (numPage < headFile.numPages) do
  begin
    if (tError > CONNECT_ERRORS) then
    begin
      StopUpdate;
      if (res = GET_RES_ERR) then
        StatusBar1.Panels.Items[0].Text := 'No connection'
      else
        StatusBar1.Panels.Items[0].Text := 'Error flashing page: ' + IntToStr(numPage + 1);
      colorStatus := clRed;
      StatusBar1.Repaint;
      FreeMem(buff_crc);
      Exit;
    end;
    StatusBar1.Panels.Items[0].Text := 'Writing page: ' + IntToStr(numPage + 1) +
                                    ' of ' + IntToStr(headFile.numPages);

    // fill one page
    CRC16 := $FFFF;
    SendPacket(CMD_FILL_BUFF, numPage, 0);
    inStream.Read(out_buf, PAGE_SIZE);
    for ix := 0 to (PAGE_SIZE - 1) do
      CRC16 := CalcCRC16(CRC16, out_buf[ix]);
    out_buf[PAGE_SIZE] := Byte(CRC16 div $100);
    out_buf[PAGE_SIZE + 1] := Byte(CRC16 mod $100);
    // transmit packet to device
    try
      ComPort1.Purge;
      ComPort1.SendBuffer(@out_buf, PAGE_SIZE + 2);
    except
      inStream.Seek(-PAGE_SIZE, soFromCurrent);
      Inc(tError);
      continue;
    end;
    res := GetPacket(Byte(ANSW_OK), Byte(ANSW_OK), 50);
    if res <> GET_RES_OK then
    begin
      inStream.Seek(-PAGE_SIZE, soFromCurrent);
      Inc(tError);
      continue;
    end;
    // write one page
    SendPacket(CMD_FLASH_PAGE, numPage, 0);
    res := GetPacket(Byte(ANSW_OK), Byte(ANSW_OK), 1000);
    if res <> GET_RES_OK then
    begin
      inStream.Seek(-PAGE_SIZE, soFromCurrent);
      Inc(tError);
      continue;
    end;
    // check CRC for one page
    Move(buff_crc[numPage * SizeOf(Word)], CRC16, SizeOf(Word));
    SendPacket(CMD_CHECK_FLASH, numPage, CRC16);
    res := GetPacket(Byte(ANSW_OK), Byte(ANSW_OK), 50);
    if res <> GET_RES_OK then
    begin
      inStream.Seek(-PAGE_SIZE, soFromCurrent);
      Inc(tError);
      continue;
    end;

    Inc(numPage);
    ProgressBar1.Position := numPage;
    tError := 0;

    Application.ProcessMessages;
    Sleep(5);
    Application.ProcessMessages;
  end;

  FreeMem(buff_crc);

  ProgressBar1.Position := 0;
  ProgressBar1.Visible := false;

  StatusBar1.Panels.Items[0].Text := 'Flashing Ok!';

  ActivateButtons(true);
  btnRefresh.Enabled := true;
  editFirmwareFile.Enabled := true;
  cbSerialPort.Enabled := true;
  Timer1.Enabled := true;
end;

end.

