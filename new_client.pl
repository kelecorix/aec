#!/usr/bin/perl

#use lib '/opt/owfs-2.8p2_static/klimat/lib';
use lib '/root/libperl/lib/perl5/site_perl/5.8.8';
#use strict;
use LWP;
use HTTP::Request::Common;

$ret = testInterneta();
if ($ret == -1) {
  exit;
}
#$ret = testServeraStat();
#if ($ret == -1) {
#  exit;
#}


@lines;
$syte_name = 'ODE-ODE-FO2';
#$file = "/var/log/send_server-klimat.log";
#$file_work = "/var/log/send_server-klimat.log.old";
$file = "/opt/owfs/freecooling/data.log";
$file_work = "/opt/owfs/freecooling/data.log.old";
$file_flag_run = "/var/run/new_client_pl.pid";

if (-e $file_flag_run) {
  print "Est flug $file_flag_run\n";
  exit;
} else {
  open(FFF,"> ".$file_flag_run);
  print FFF "run\n";
}

if (!-e $file_work) {
  print "Отправленного файла нету подготовим файл\n";
  if (-e $file) {
    rename($file, $file_work); 
  }
} else {
  print "Есть не отправленый файл отправим\n";
  if (-e $file_flag_run) {
    unlink($file_flag_run);
  }  
}


if (-e $file_work) {
  #print "The file $file is valid and exists!";
  open(FILE_W,$file_work);

  while (<FILE_W>) {
    #$line_tmp = fgets($fh, 4096);
    $line_tmp = $_;
    chop $line_tmp;
    #print "$line_tmp\n";
    #$line_tmp = str_replace('\r\n', '', $line_tmp);
    #$line_tmp = str_replace('\n', '', $line_tmp);
    if (length($line_tmp) > 5) {
      push @lines, $line_tmp;
    }
  }
  # Выполнить операции с файлом
  
  close(FILE_W); 

  $i = 0;  
  $str_send_tmp = '';
  foreach $line (@lines) {
    print $line."\n"; 
    #@expl_tmp = split(/\|/,$line);
    #$str_send_tmp = $str_send_tmp.$expl_tmp[0].' '.$expl_tmp[1].'!';
    $str_send_tmp = $str_send_tmp.$line.'!';
    print "$str_send_tmp\n";
    $i++;
    if ($i == 20) {
    #  //отправим на сервер
      chop $str_send_tmp;

      $ret = -1;
      print "STR SEND: \n".$str_send_tmp."\n";
      while ($ret == -1) {
        $ret = SendServer($str_send_tmp);
        print "Ретурн отправка ".$ret."\n";
        sleep(5);
      }        

      print $str_send_tmp;
      $str_send_tmp = '';
      $i = 0;
      print "\n\n";
    }  
    
  }

  if (length($str_send_tmp) > 5) {
    print "Остаток для отсылки\n";
    chop $str_send_tmp;
    print $str_send_tmp;
    print "\n\n";
    #//отправим на сервер остаток
    $ret = -1;
    while ($ret == -1) {
      $ret = SendServer($str_send_tmp);
      print "Ретурн отправка ".$ret."\n";
      sleep(5);
    }
                  
  }
  
  print "Все отправили удалим файл\n";      
  unlink($file_work);
  unlink($file_flag_run);
    
} else {
  print "Файла для отправки нету\n";
}

sub SendServer {
  my $send_str = shift;
  #my $data = "sayt=od0003&data=".$send_str;
  my $ua = LWP::UserAgent->new();
  $ua->agent("PerlUA/0.1");

  my $url = "http://195.138.71.229:443/new_server.php";
  my $request = POST $url, [sayt=> $syte_name, data => $send_str];
  my $document = $ua->request($request);
  if ($document->is_success) {
    print "Content-Type: text/html\n\n";
    print $document->content;
    return 1;
  } else {
    print "Content-Type: text/html\n\n";
    print "Couldn't post to $url\n";
    return -1;
  }
  
}

sub testServeraStat {
 $ff = system "ping -c 2 -q -w 5 195.138.71.229 | grep packets > /tmp/test.ping"; 

  open(TESTPING, "/tmp/test.ping");
  $test_ping = <TESTPING>;
  chomp $test_ping;
  close TESTPING;
  unlink "/tmp/test.ping";
  @lTePing = split(/, /,$test_ping);
  @lTePing_0  = split(/ /,$lTePing[0]);
  @lTePing_1  = split(/ /,$lTePing[1]);
  @lTePing_2  = split(/ /,$lTePing[2]);
  #print $test_ping."\n";
  #print $lTePing[0]."\n";
  #print $lTePing_0[0]."\n";
  #print $lTePing[1]."\n";
  #print $lTePing_1[0]."\n";
  #print $lTePing[2]."\n";
  #print $lTePing_2[0]."\n";
  #print $lTePing[3]."\n";

  if ($lTePing_1[0] > 0) {
    print "Server poka imeetsya\n";
    return 1;
  } else {
    print "Servera net\n";
    return -1
  }
}
sub testInterneta {

  $ff = system "ping -c 2 -q -w 5 195.138.80.33 | grep packets > /tmp/test.ping";
  #$ff = system "ping -c 2 -q -w 5 195.138.79.33 | grep packets > /tmp/test.ping";
  #3 packets transmitted, 2 received, 33% packet loss, time 2018ms        
  #2 packets transmitted, 2 received, 0% packet loss, time 2018ms
  #my @timeLog = tekDate();
  #print $timeLog[0]." ".$timeLog[1]."\n";

  open(TESTPING, "/tmp/test.ping");
  $test_ping = <TESTPING>;
  chomp $test_ping;
  close TESTPING;
  unlink "/tmp/test.ping";
  @lTePing = split(/, /,$test_ping);
  @lTePing_0  = split(/ /,$lTePing[0]);
  @lTePing_1  = split(/ /,$lTePing[1]);
  @lTePing_2  = split(/ /,$lTePing[2]);

  #print $dd."\n";
  #print $lTePing[0]."\n";
  #print $lTePing_0[0]."\n";
  #print $lTePing[1]."\n";
  #print $lTePing_1[0]."\n";
  #print $lTePing[2]."\n";
  #print $lTePing_2[0]."\n";
  #print $lTePing[3]."\n";
  if ($lTePing_1[0] > 0) {
    print "Internet poka imeetsya\n";
    return 1;
  } else {
    print "Interneta net\n";

    #$screenPPP = `ps ax | grep -v grep | grep -m 1 -c \"SCREEN -dmS ppp\" | grep -v grep`;
    #$ppp_on = `ps ax | grep -v grep | grep -m 1 -c \"ppp-on\" | grep -v grep`;
    #$pppd = `ps ax | grep -v grep | grep -m 1 -c \"pppd\" | grep -v grep`;
    #print "screenPPP = ", $screenPPP, "ppp_on = ", $ppp_on, "pppd = ", $pppd, "\n";

    $dd = `/usr/bin/pkill -9 /usr/bin/wvdial`;
    sleep 1;
    $dd = `/usr/bin/killall -9 /usr/bin/wvdial`;
    
    system("echo '0=1' > /dev/pi-blaster");
    sleep 5;
    system("echo '0=0' > /dev/pi-blaster");
    sleep 30;
    
    #if ($screenPPP == 1) {
    #  $pidScreenPPP = `ps ax | grep -v grep | grep -m 1 \"SCREEN -dmS ppp\" | grep -v grep`;
    #  chomp $pidScreenPPP;
    #  #@lpidScreenPPP = split(/ /,$pidScreenPPP);
    #  @lpidScreenPPP= $pidScreenPPP=~m/(\d+\.?\d*|\.\d+)/g;
    #  $kk = `kill -9 $lpidScreenPPP[0]`;
      sleep 2;
      print "run PPP-ON\n";
    #  $dd = `/usr/bin/screen -dmS ppp /etc/ppp/ppp-on`;
       $dd = `/usr/bin/screen -dmS wvdial /usr/bin/wvdial`;
      #$ff = system "/usr/local/perevod/bin/perevodNashServer.pl 0 0 0 0 $terNumber 0 0 0 $configFiles 102 \"screen ppp runing\" &";
      
    #} else {

      #print "run PPP-ON\n";
      #$dd = `/usr/bin/screen -dmS ppp /etc/ppp/ppp-on`;
      #  $ff = system "/usr/local/perevod/bin/perevodNashServer.pl 0 0 0 0 $terNumber 0 0 0 $configFiles 102 \"screen ppp runing\" &";
  
    #}
    return -1;
  }
}

#function SendServer($send_str) {
#  $socket = fsockopen('sayts.1gb.ua', 80, $errno, $errstr, 120);
#
#  //print "Мы в функции".$send_str."\n"; 
#  //if(!$socket)die("$errstr($errno)");
#  if(!$socket) {
#    
#    return -1;  
#  
#  } else {
#    $data = "sayt=".urlencode("od22345")."&data=".urlencode($send_str);
#    //print "Мы в функции".$data."\n";   
#    fwrite($socket, "POST /new_server.php HTTP/1.1\r\n");
#    fwrite($socket, "Host: sayts.1gb.ua\r\n");
#    
#    fwrite($socket,"Content-type: application/x-www-form-urlencoded\r\n");
#    fwrite($socket,"Content-length:".strlen($data)."\r\n");
#    fwrite($socket,"Accept:*/*\r\n");
#    fwrite($socket,"User-agent:Opera 10.00\r\n");
#    fwrite($socket,"\r\n");
#     
#    fwrite($socket,"$data\r\n");
#    fwrite($socket,"\r\n");
#      
#    $answer = '';
#    while(!feof($socket)){
#      $answer.= fgets($socket, 4096);
#    }
#    echo $answer;
#  }
#          
#  fclose($socket);
#  return 1;
#}
