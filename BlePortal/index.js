const decoder = new TextDecoder('utf-8');
const genericAccess = '00001800-0000-1000-8000-00805f9b34fb';
const btnService = '5e87bd74-b350-4a6a-ae36-bb33e30499af';
const btnChar = '12cd634c-2911-45c5-8c46-3d67628a88fd';
const dataChar = '2e550f9f-c882-467a-b35a-c7f12ecae2b3';

var bleDevice = null;

function onDevice()
{
	clearLog();
	setDisabled("connect", true);	
	setDisplay("dataInput", "none");
	setContent("deviceName", "Connecting...");
	return navigator.bluetooth.requestDevice(
		{
			acceptAllDevices: true,
			optionalServices: [btnService]
		})
		.then(device => {
			bleDevice = device;
			setDisabled("connect", false);
			setDisabled("writeButton", false);
			setDisplay("dataInput", "inline");
			setContent("deviceName", "Device ID: " + bleDevice.id);
			setValue("writeData", bleDevice.name);
			bleDevice.addEventListener('gattserverdisconnected', onDisconnected);
		})
		.then(any => initChar(btnChar, handleBtn))
		.then(any => initChar(dataChar, handleData))
		.then(any => readChar(dataChar, setData))
		.catch(error => 
			{
				clearLog();
				console.log(error.message)
			});
}

function onDisconnected()
{
	clearLog();
	log('Bluetooth Device disconnected');
}

function setDisplay(id, display)
{
	document.getElementById(id).style.display = display;
}

function setDisabled(id, disabled)
{
	document.getElementById(id).disabled = disabled;
}

function setContent(id, text)
{
	document.getElementById(id).innerHTML = text;
}

function setValue(id, text)
{
	document.getElementById(id).value = text;
}

function initChar(char, handle)
{
	return getChar(char)
	.then(ch => subscribe(ch, handle))
	.then(ch => {
		console.log("Subscribed to " + char);
		return ch;
	});
}

function getChar(characteristic)
{	
	return bleDevice.gatt.connect()
	.then(server => server.getPrimaryService(btnService))
	.then(service => service.getCharacteristic(characteristic))
}

function readChar(char, handle)
{
	return getChar(char)
	.then(ch => ch.readValue())
	.then(value => decoder.decode(value))
	.then(result => handle(result));
}

function subscribe(char, handle)
{
	return char.startNotifications()
	.then(char => char.addEventListener('characteristicvaluechanged', handle));
}

function handleBtn(event)
{
	log("Button state is " + event.target.value.getUint8(0));
}

function handleData(event)
{
	setData(decoder.decode(event.target.value));
}

function setData(value)
{
	return document.getElementById('data').innerHTML = value;
}

function onRename()
{
	let encoder = new TextEncoder('utf-8');
	let data = encoder.encode(document.getElementById('writeData').value);
	
	getChar(dataChar)
	.then(ch => ch.writeValue(data))
	.then(any => setDisabled("writeButton", true))
	.catch(error => console.log(error));
}

function log() {
	var line = Array.prototype.slice.call(arguments).map(function (argument) {
			return typeof argument === 'string' ? argument : JSON.stringify(argument);
		}).join(' ');
	
	document.querySelector('#log').textContent += line + '\n';
}

function clearLog() {
	document.querySelector('#log').textContent = '';
	setData('');
	setContent("deviceName", "");
	setDisplay("dataInput", "none");
	setDisabled("connect", false);
}