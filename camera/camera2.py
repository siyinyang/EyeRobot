import urllib
import urllib2
import time

def control(num):
	data = {}
	data['user'] = 'admin' 
	data['pwd'] = '' 
	data['command'] = num 
	url_values = urllib.urlencode(data)  
	url = 'http://192.168.1.145:1234/decoder_control.cgi' 
	full_url = url + '?' + url_values
	data = urllib2.urlopen(full_url)

def snapshot(n)
	data = {}
	data['user'] = 'admin' 
	data['pwd'] = '' 
	data['next_url'] = n 
	url_values = urllib.urlencode(data)  
	url = 'http://192.168.1.145:1234/snapshot.cgi' 
	full_url = url + '?' + url_values
	data = urllib2.urlopen(full_url)

#main rountine
control(31) #set to preset1
time.sleep(15) #wait 15 seconds
snapshot(0);#take snapshot

#iterate every preset to take snapshots
for n in range(33, 53, 2):
	control(n)
	time.sleep(5)
	snapshot(n/2-15)

