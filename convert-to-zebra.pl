#!/usr/bin/perl -w

die "Missing Arguments.\n usage: ./convert-to-wsgow.pl <Input_File_Name> <Directory_Name>\n\n" unless (defined $ARGV[0] && defined $ARGV[1]);

open INFILE, "<$ARGV[0]"; #nombre del archivo

$filecounter=1;
$newfile=1;

if (!-d "./$ARGV[1]") #si no existe carpeta la creara
{
    `mkdir $ARGV[1]`;
}



while (<INFILE>) #mientras no se termine el archivo
{
    if ($newfile)
    {
        open OUTFILE, ">./$ARGV[1]/$filecounter.txt";
        $filecounter++;
        $newfile=0;
    }
    
    if ($_ =~ /(\d+), -(\d+)/)
    {
        print OUTFILE ($1*1000)."\t".(-$2).".0\n";
    }
    elsif ($_ =~ /gps, (\d+), (\d+\.\d+), (\d+)\.(\d+), (\d+)\.(\d+)/)
    {
        $minutelat=$3/100.0;
        $declat="0.".$4;
        $lat=int($minutelat)+(100*($minutelat-int($minutelat))+$declat)/60.0;

        $minutelon=$5/100.0;
        $declon="0.".$6;
        $lon=int($minutelon)+(100*($minutelon-int($minutelon))+$declon)/60.0;
    
        print OUTFILE "$lat\n-$lon\n";
        
        print OUTFILE substr($1,0,2)."/".substr($1,2,2)."/".substr($1,4,2);
        print OUTFILE " ".substr($2,0,2).":".substr($2,2,2).":".substr($2,4,5)."\n";
        close OUTFILE;
        $newfile=1;
    }
    else
    {
        die "Error: There is an inconsistency in the input file ** $ARGV[1] **\n";
    }
}
