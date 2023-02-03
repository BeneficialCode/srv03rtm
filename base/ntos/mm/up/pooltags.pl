
#Copyright (c) 1992-2000  Microsoft Corporation
#
#Module Name:
#
#    pooltags.pl
#
#Abstract:
#
#    WinDbg Extension Api
#
#Environment:
#
#    User Mode.
#
#Revision History:
# 
#    Kshitix K. Sharma (kksharma)
#        
#
# Parse pooltags.w file to eliminate owner info and generate public pooltag.txt
#
# PPPP - <One line description string>
#
#     PPPP is a 4 character max pooltag
#
# Any line which doesn't fit this format is ignored
#

sub EmitFileHeader;
sub NextLine;
sub IsPoolTagDescription;

#####################################################################################
#
# main
#
#      Usage pooltags [-o <outfile>] [-i] <infile> 
#
#####################################################################################
$g_NumLine = 0;
$PoolTagDesc = {
   TAG           => 0,
   DESCRIPTION   => "",
   };

while ($arg = shift) {
   if ($arg eq "-o") {
      $OutFileName = shift;
   } elsif ($arg eq "-i") {
      $PoolTagTxtFile = shift;
   } else {
      $PoolTagTxtFile = $arg;
   }
}

die "Cannot open file $PoolTagTxtFile\n" if !open(POOLT_FILE, $PoolTagTxtFile);
die "Cannot open file $OutFileName\n" if !open(OUT_FILE, ">" . $OutFileName);

EmitFileHeader();
while (IsPoolTagDescription() || NextLine ) {
   $LineToPrint = $g_line;
   if (($PoolTag,$Description) = $g_line =~ /^ ?(....) +-\s*(.*)$/) {
      ($T1,$T2,$T3,$T4) = $PoolTag =~ /^(.)(.)(.)(.)$/;
      if (($RestOfLine, $Owner) = $g_line =~ /(.*)- OWNER (.*)/) {
         # We have owner info here                
         $LineToPrint = $RestOfLine . "\n";
      }
   }
   printf OUT_FILE ($LineToPrint);
 
} continue {
   close POOLT_FILE if eof;
}

#####################################################################################
#
# Subroutines
#
#####################################################################################


sub NextLine {
   $g_line = <POOLT_FILE>;
   $g_NumLine++;
   return $g_line;
}
   
sub IsPoolTagDescription {
   # Match PPPP - <Description>
   if ($g_line =~ /^ ?.... +- .*$/) { 
      return 0;
   }
   if ($g_line =~ /^([A-Z][A-Z_0-9]+)\s*\((.*)\)$/) {
      return 1;
   }
   return 0;
}

sub EmitFileHeader {

   print OUT_FILE "REM -------------------------------------".
        "--------------------------------------\n";
   print OUT_FILE "REM\n";
   print OUT_FILE "REM   Public pooltag.txt\n";
   print OUT_FILE "REM\n";
   print OUT_FILE "REM IMPORTANT:  This file is automatically generated.\n";
   print OUT_FILE "REM             Do not edit by hand.\n";
   print OUT_FILE "REM\n";
   print OUT_FILE "REM Generated from $PoolTagTxtFile on " . localtime() . "\n";
   print OUT_FILE "REM\n";
   print OUT_FILE "REM-------------------------------------".
       "--------------------------------------\n\n";
   print OUT_FILE "\n\n";
}
