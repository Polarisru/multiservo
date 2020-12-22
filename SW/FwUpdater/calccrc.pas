unit CalcCRC;

{$mode objfpc}{$H+}

interface

function CalcCRC32(crc: LongWord; octet: Byte) : LongWord;
function CalcCRC16(crc: Word; data: Byte): Word;

implementation

const crc_32_tab: array[0..255] of LongWord = (
  $00000000, $77073096, $ee0e612c, $990951ba, $076dc419, $706af48f, $e963a535, $9e6495a3,
  $0edb8832, $79dcb8a4, $e0d5e91e, $97d2d988, $09b64c2b, $7eb17cbd, $e7b82d07, $90bf1d91,
  $1db71064, $6ab020f2, $f3b97148, $84be41de, $1adad47d, $6ddde4eb, $f4d4b551, $83d385c7,
  $136c9856, $646ba8c0, $fd62f97a, $8a65c9ec, $14015c4f, $63066cd9, $fa0f3d63, $8d080df5,
  $3b6e20c8, $4c69105e, $d56041e4, $a2677172, $3c03e4d1, $4b04d447, $d20d85fd, $a50ab56b,
  $35b5a8fa, $42b2986c, $dbbbc9d6, $acbcf940, $32d86ce3, $45df5c75, $dcd60dcf, $abd13d59,
  $26d930ac, $51de003a, $c8d75180, $bfd06116, $21b4f4b5, $56b3c423, $cfba9599, $b8bda50f,
  $2802b89e, $5f058808, $c60cd9b2, $b10be924, $2f6f7c87, $58684c11, $c1611dab, $b6662d3d,
  $76dc4190, $01db7106, $98d220bc, $efd5102a, $71b18589, $06b6b51f, $9fbfe4a5, $e8b8d433,
  $7807c9a2, $0f00f934, $9609a88e, $e10e9818, $7f6a0dbb, $086d3d2d, $91646c97, $e6635c01,
  $6b6b51f4, $1c6c6162, $856530d8, $f262004e, $6c0695ed, $1b01a57b, $8208f4c1, $f50fc457,
  $65b0d9c6, $12b7e950, $8bbeb8ea, $fcb9887c, $62dd1ddf, $15da2d49, $8cd37cf3, $fbd44c65,
  $4db26158, $3ab551ce, $a3bc0074, $d4bb30e2, $4adfa541, $3dd895d7, $a4d1c46d, $d3d6f4fb,
  $4369e96a, $346ed9fc, $ad678846, $da60b8d0, $44042d73, $33031de5, $aa0a4c5f, $dd0d7cc9,
  $5005713c, $270241aa, $be0b1010, $c90c2086, $5768b525, $206f85b3, $b966d409, $ce61e49f,
  $5edef90e, $29d9c998, $b0d09822, $c7d7a8b4, $59b33d17, $2eb40d81, $b7bd5c3b, $c0ba6cad,
  $edb88320, $9abfb3b6, $03b6e20c, $74b1d29a, $ead54739, $9dd277af, $04db2615, $73dc1683,
  $e3630b12, $94643b84, $0d6d6a3e, $7a6a5aa8, $e40ecf0b, $9309ff9d, $0a00ae27, $7d079eb1,
  $f00f9344, $8708a3d2, $1e01f268, $6906c2fe, $f762575d, $806567cb, $196c3671, $6e6b06e7,
  $fed41b76, $89d32be0, $10da7a5a, $67dd4acc, $f9b9df6f, $8ebeeff9, $17b7be43, $60b08ed5,
  $d6d6a3e8, $a1d1937e, $38d8c2c4, $4fdff252, $d1bb67f1, $a6bc5767, $3fb506dd, $48b2364b,
  $d80d2bda, $af0a1b4c, $36034af6, $41047a60, $df60efc3, $a867df55, $316e8eef, $4669be79,
  $cb61b38c, $bc66831a, $256fd2a0, $5268e236, $cc0c7795, $bb0b4703, $220216b9, $5505262f,
  $c5ba3bbe, $b2bd0b28, $2bb45a92, $5cb36a04, $c2d7ffa7, $b5d0cf31, $2cd99e8b, $5bdeae1d,
  $9b64c2b0, $ec63f226, $756aa39c, $026d930a, $9c0906a9, $eb0e363f, $72076785, $05005713,
  $95bf4a82, $e2b87a14, $7bb12bae, $0cb61b38, $92d28e9b, $e5d5be0d, $7cdcefb7, $0bdbdf21,
  $86d3d2d4, $f1d4e242, $68ddb3f8, $1fda836e, $81be16cd, $f6b9265b, $6fb077e1, $18b74777,
  $88085ae6, $ff0f6a70, $66063bca, $11010b5c, $8f659eff, $f862ae69, $616bffd3, $166ccf45,
  $a00ae278, $d70dd2ee, $4e048354, $3903b3c2, $a7672661, $d06016f7, $4969474d, $3e6e77db,
  $aed16a4a, $d9d65adc, $40df0b66, $37d83bf0, $a9bcae53, $debb9ec5, $47b2cf7f, $30b5ffe9,
  $bdbdf21c, $cabac28a, $53b39330, $24b4a3a6, $bad03605, $cdd70693, $54de5729, $23d967bf,
  $b3667a2e, $c4614ab8, $5d681b02, $2a6f2b94, $b40bbe37, $c30c8ea1, $5a05df1b, $2d02ef8d
);

const crc16_tab: array[0..255] of Word = (
  $0000,$8005,$800F,$000A,$801B,$001E,$0014,$8011,$8033,$0036,$003C,$8039,$0028,$802D,$8027,$0022,
  $8063,$0066,$006C,$8069,$0078,$807D,$8077,$0072,$0050,$8055,$805F,$005A,$804B,$004E,$0044,$8041,
  $80C3,$00C6,$00CC,$80C9,$00D8,$80DD,$80D7,$00D2,$00F0,$80F5,$80FF,$00FA,$80EB,$00EE,$00E4,$80E1,
  $00A0,$80A5,$80AF,$00AA,$80BB,$00BE,$00B4,$80B1,$8093,$0096,$009C,$8099,$0088,$808D,$8087,$0082,
  $8183,$0186,$018C,$8189,$0198,$819D,$8197,$0192,$01B0,$81B5,$81BF,$01BA,$81AB,$01AE,$01A4,$81A1,
  $01E0,$81E5,$81EF,$01EA,$81FB,$01FE,$01F4,$81F1,$81D3,$01D6,$01DC,$81D9,$01C8,$81CD,$81C7,$01C2,
  $0140,$8145,$814F,$014A,$815B,$015E,$0154,$8151,$8173,$0176,$017C,$8179,$0168,$816D,$8167,$0162,
  $8123,$0126,$012C,$8129,$0138,$813D,$8137,$0132,$0110,$8115,$811F,$011A,$810B,$010E,$0104,$8101,
  $8303,$0306,$030C,$8309,$0318,$831D,$8317,$0312,$0330,$8335,$833F,$033A,$832B,$032E,$0324,$8321,
  $0360,$8365,$836F,$036A,$837B,$037E,$0374,$8371,$8353,$0356,$035C,$8359,$0348,$834D,$8347,$0342,
  $03C0,$83C5,$83CF,$03CA,$83DB,$03DE,$03D4,$83D1,$83F3,$03F6,$03FC,$83F9,$03E8,$83ED,$83E7,$03E2,
  $83A3,$03A6,$03AC,$83A9,$03B8,$83BD,$83B7,$03B2,$0390,$8395,$839F,$039A,$838B,$038E,$0384,$8381,
  $0280,$8285,$828F,$028A,$829B,$029E,$0294,$8291,$82B3,$02B6,$02BC,$82B9,$02A8,$82AD,$82A7,$02A2,
  $82E3,$02E6,$02EC,$82E9,$02F8,$82FD,$82F7,$02F2,$02D0,$82D5,$82DF,$02DA,$82CB,$02CE,$02C4,$82C1,
  $8243,$0246,$024C,$8249,$0258,$825D,$8257,$0252,$0270,$8275,$827F,$027A,$826B,$026E,$0264,$8261,
  $0220,$8225,$822F,$022A,$823B,$023E,$0234,$8231,$8213,$0216,$021C,$8219,$0208,$820D,$8207,$0202
);

function CalcCRC32(crc: LongWord; octet: Byte) : LongWord;
begin
  crc := crc_32_tab[Byte(crc xor Longword(octet))] xor ((crc shr 8) and $00FFFFFF);
  Result := crc;
end;

function CalcCRC16(crc: Word; data: Byte): Word;
var
   index: Byte;
begin
  index := crc shr 8;
  crc := crc shl 8;
  crc := crc xor crc16_tab[index xor data];
  Result := crc;
end;

end.

