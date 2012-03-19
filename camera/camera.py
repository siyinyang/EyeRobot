import sys
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

def snapshot(n, path):
	data = {}
	data['user'] = 'admin' 
	data['pwd'] = '' 
	data['next_url'] = n 
	url_values = urllib.urlencode(data)  
	url = 'http://192.168.1.145:1234/snapshot.cgi' 
	full_url = url + '?' + url_values
	outpath = path + "/" + str(n) + ".jpg"
	#data = urllib2.urlopen(full_url)
	urllib.urlretrieve(full_url, outpath)

def main(path):
	#main rountine
	control(31) #set to preset1
	time.sleep(25) #wait 25 seconds
	snapshot(0);#take snapshot

	#iterate every preset to take snapshots
	for n in range(33, 53, 2):
		control(n)
		time.sleep(5)
		snapshot(n/2-15, path)

if __name__ = "__main__":
	path = sys.argv[1]
	main(path)

