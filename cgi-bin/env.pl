#!/usr/bin/perl

print "Content-type: text/html\r\n";
print "\r\n";

print "<html><head></head><body><ul>\n";
foreach (sort keys %ENV)
{
  print "<li>$_: $ENV{$_}</li>\n";
}
print "</ul></body></html>";
1;
