// ignore_for_file: avoid_print, non_constant_identifier_names
import 'package:flutter/material.dart';
import 'dart:async';
import 'dart:convert' show utf8;
import 'package:flutter_blue/flutter_blue.dart';
import 'package:webview_flutter/webview_flutter.dart';

void main() {
  runApp(const MaterialApp(
    title: 'Choose route',
    home: ChooseRoute(),
  ));
}

class ChooseRoute extends StatelessWidget {
  const ChooseRoute({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xff033433),
      appBar: AppBar(
        centerTitle: true,
        title: const Text('Izbirni meni'),
        backgroundColor: Colors.cyan[900],
      ),
      body: SizedBox(
        width: MediaQuery.of(context).size.width,
        child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              SizedBox(
                width: 200,
                height: 100,
                child: ElevatedButton(
                  style: ElevatedButton.styleFrom(
                    primary: Colors.cyan[900],
                    onPrimary: Colors.black,
                  ),
                  child: const Text(
                    'BluLock',
                    style: TextStyle(
                      fontSize: 17,
                      color: Colors.white,
                    ),
                  ),
                  onPressed: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(builder: (context) => const BluLock()),
                    );
                  },
                ),
              ),
              const SizedBox(
                height: 10,
              ),
              SizedBox(
                width: 200,
                height: 100,
                child: ElevatedButton(
                  style: ElevatedButton.styleFrom(
                    primary: Colors.cyan[900],
                    onPrimary: Colors.white,
                  ),
                  child: const Text(
                    'WebView',
                    style: TextStyle(
                      fontSize: 17,
                      color: Colors.white,
                    ),
                  ),
                  onPressed: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(builder: (context) => const VebView()),
                    );
                  },
                ),
              ),
            ]),
      ),
    );
  }
}

//v main je drgac samo runApp(BluLock());

class BluLock extends StatelessWidget {
  const BluLock({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      color: Colors.lightBlue,
      initialRoute: '/',
      routes: {
        '/': (context) => const MainActivity(),
      },
    );
  }
}

class MainActivity extends StatefulWidget {
  const MainActivity({Key? key}) : super(key: key);

  @override
  _MainActivityState createState() => _MainActivityState();
}

class _MainActivityState extends State<MainActivity> {
  final String SERVICE_UUID = "def8570d-1d17-4908-8bcb-082e56f566b9";
  final String CHARACTERISTIC_UUID = "f1226eca-6253-40e6-9c8b-1e0e5406e689";
  final String TARGET_DEVICE_NAME = "ESP32 get noti from device";

  FlutterBlue flutterBlue = FlutterBlue.instance;
  StreamSubscription<ScanResult>? scanSubScription;

  BluetoothDevice? targetDevice;
  BluetoothCharacteristic? targetCharacteristic;

  String conncetionText = "";

  @override
  void initState() {
    super.initState();
    startScan();
  }

  startScan() {
    print("starting scan");
    setState(() {
      conncetionText = "Start scanning";
    });

    scanSubScription = flutterBlue.scan().listen((scanResult) {
      if (scanResult.device.name == TARGET_DEVICE_NAME) {
        print("DEVICE FOUND");
        stopScan();
        setState(() {
          conncetionText = "Found Target Device!";
        });

        targetDevice = scanResult.device;
        connectToDevice();
      }
    }, onDone: () => stopScan());
  }

  stopScan() {
    flutterBlue.stopScan();
    scanSubScription?.cancel();
    scanSubScription = null;
  }

  connectToDevice() async {
    if (targetDevice == null) return;

    setState(() {
      conncetionText = "Device Connecting...";
    });

    await targetDevice?.connect();
    print("DEVICE CONNECTED");
    setState(() {
      conncetionText = "Device Connected!";
    });

    discoverServices();
  }

  disconnectFromDevice() {
    if (targetDevice == null) return;

    targetDevice?.disconnect();

    setState(() {
      conncetionText = "Device Disconnected ;(";
    });
  }

  discoverServices() async {
    if (targetDevice == null) return;

    List<BluetoothService>? services = await targetDevice?.discoverServices();
    services?.forEach((service) {
      if (service.uuid.toString() == SERVICE_UUID) {
        for (var characteristic in service.characteristics) {
          if (characteristic.uuid.toString() == CHARACTERISTIC_UUID) {
            targetCharacteristic = characteristic;
            writeData("Hi thereeeeee, esp");
            setState(() {
              conncetionText = "All Ready With ${targetDevice?.name}";
            });
          }
        }
      }
    });
  }

  writeData(String data) async {
    if (targetCharacteristic == null) return;

    List<int> bytes = utf8.encode(data);
    targetCharacteristic?.write(bytes);
  }

  @override
  Widget build(BuildContext kontext) {
    return MaterialApp(
      theme: ThemeData(scaffoldBackgroundColor: const Color(0xff033433)),
      home: Scaffold(
        appBar: AppBar(
          centerTitle: true,
          title: const Text('Bluetooth lock'),
          backgroundColor: Colors.cyan[900],
          leading: GestureDetector(
            onTap: () {
              stopScan();
              disconnectFromDevice();
              Navigator.of(
                context,
                rootNavigator: true,
              ).pop(
                context,
              );
            },
            child: const Icon(
              Icons.arrow_back, // add custom icons also
            ),
          ),
        ),
        body: SizedBox(
          width: MediaQuery.of(context).size.width,
          child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                SizedBox(
                  width: 200,
                  height: 100,
                  child: ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      primary: Colors.greenAccent,
                      onPrimary: Colors.white,
                    ),
                    child: const Text(
                      'Odkleni',
                      style: TextStyle(
                        fontSize: 17,
                        color: Colors.black,
                      ),
                    ),
                    onPressed: () => writeData("1"),
                  ),
                ),
                const SizedBox(
                  height: 10,
                ),
                SizedBox(
                  width: 200,
                  height: 100,
                  child: ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      primary: Colors.redAccent,
                      onPrimary: Colors.white,
                    ),
                    child: const Text(
                      'Zakleni',
                      style: TextStyle(
                        fontSize: 17,
                        color: Colors.black,
                      ),
                    ),
                    onPressed: () => writeData("0"),
                  ),
                ),
                const SizedBox(
                  height: 60,
                ),
                /* Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    ElevatedButton(
                      onPressed: () => connectToDevice(),
                      child: const Text("connect"),
                    ),
                    ElevatedButton(
                      onPressed: () => disconnectFromDevice(),
                      child: const Text("disconnect"),
                    ),
                  ],
                ), */
              ]),
        ),
      ),
    );
  }
}

class VebView extends StatelessWidget {
  const VebView({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: const Text('Wifi Lock'),
          centerTitle: true,
          backgroundColor: Colors.cyan[900],
          leading: GestureDetector(
            onTap: () {
              Navigator.pop(context);
            },
            child: const Icon(
              Icons.arrow_back, // add custom icons also
            ),
          ),
        ),
        body: const WebView(
          javascriptMode: JavascriptMode.unrestricted,
          initialUrl: '192.168.200.2',
        ));
  }
}
