# Projet_IOC_ZHANG_WANG



## Objectif:
Dans ce projet, nous avons créé un système permettant de surveiller des capteurs ESP32 via un navigateur Web. Les principales étapes du projet comprennent la création d'une application ESP32, la configuration d'un MQTT broker sur un Raspberry Pi, la création d'une application passerelle et d'un serveur HTTP sur le Raspberry Pi, et enfin le test et l'optimisation du système.

## 1. Création de l'application ESP32
Cette partie de code se trouve dans le fichier *projet.ino*.
### Connexion Wifi
On a d'abord écrit le module ESP32 pour qu'il se connecte à un réseau Wi-Fi. On a utilisé un point d'accès Wi-Fi créé par un téléphone portable pour fournir une connexion Internet. Cela permet au module ESP32 de se connecter à Internet et de communiquer avec le broker MQTT sur le Raspberry Pi. On a utilisé un boucle while dans le setup() pour vérifier si la connexion Wi-Fi est établie. Si ce n'est pas le cas, elle attend 1 seconde et refait le connexion.
```
#include <WiFi.h>
//Définition des identifiants Wi-Fi
const char* ssid = "tp";
const char* password = "xmyz02200059Y";

void setup() {
      Serial.begin(115200);
      WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED) {
          delay(1000);
          Serial.println("Connecting to Wi-Fi...");
      }
      Serial.println("Connected to Wi-Fi");
}
void loop() {
}
```

### Client MQTT
Nous avons configuré un client MQTT sur l'ESP32 pour publier les données des capteurs vers un broker MQTT.

Les variables ci-dessous définissent l'adresse IP du serveur MQTT, le port, l'utilisateur et le mot de passe pour se connecter au serveur MQTT. On a créé un objet WiFiClient pour gérer la connexion Wi-Fi et pour initialiser l'objet PubSubClient, qui gérera la communication avec le serveur MQTT.
```
#include <PubSubClient.h>
//Configuration des paramètres MQTT
const char* mqttServer = "172.20.10.12";
const int mqttPort = 1883;
const char* mqttUser = "wz";
const char* mqttPassword = "123456";
//Création des objets client
WiFiClient espClient;
PubSubClient client(espClient);
```
On a utilisé la fonction setServer() pour configurer les paramètres du serveur MQTT, et la fonction setCallback() qui sera appelée lors de la réception d'un message MQTT. Ensuite, le code se connecte au serveur MQTT en utilisant les informations d'identification définies précédemment.
```
//Connexion au serveur MQTT
client.setServer(mqttServer, mqttPort);
client.setCallback(callback);

while (!client.connected()) {
  Serial.println("Connecting to MQTT broker...");

  if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
    Serial.println("Connected to MQTT broker");
  } else {
    Serial.print("Failed to connect, error state: ");
    Serial.println(client.state());
    delay(2000);
  }
}
```
On lit ensuite la valeur du capteur photorésistance et la convertit en une chaîne de caractères. Après, on publie la valeur sur le serveur MQTT en utilisant le topic "testTopic".

```
int photoResistorValue = analogRead(photoResistorPin);
char msg[10];
snprintf(msg, 10, "%d", photoResistorValue);
client.publish("testTopic", msg);
```
Enfin, la fonction de rappel est appelée lorsqu'une message MQTT est reçu sur le topic "testTopic", le code vérifie si le message reçu est "ON" ou "OFF" et modifie l'état de la LED en conséquence. (On n'a pas eu le temps pour tester le contrôle du led à la fin, mais on a gardé cette partie de code.)
```
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "testTopic") {
    if (message == "ON") {
      digitalWrite(ledPin, HIGH);
    } else if (message == "OFF") {
      digitalWrite(ledPin, LOW);
    }
  }
}
```

Pour l'instant, on n'a pas encore configuré le brocker MQTT sur le Raspberry Pi.
![](https://i.imgur.com/uUV2kbK.png)
Si on néglige la partie de MQTT, on voit bien des valeurs mesurées par le capteur.
![](https://i.imgur.com/aWkSbsQ.png)




## 2. Configuration du broker MQTT sur le Raspberry Pi

On a d'abord installé le Raspbian Lite dans le Raspberry Pi. Voici le tutoriel qu'on a suivi pour l'installation : [Install Raspberry Pi OS](https://randomnerdtutorials.com/installing-raspbian-lite-enabling-and-connecting-with-ssh/)
Ensuite, on a installé Mosquitto MQTT Broker sur Raspberry Pi.
**Installation Mosquitto Broker sur Raspberry Pi**
```
sudo apt-get update
// To install the Mosquitto Broker 
sudo apt-get install -y mosquitto mosquitto-clients
//To make Mosquitto auto start when the Raspberry Pi boots (the Mosquitto broker will automatically start when the Raspberry Pi starts)
sudo systemctl enable mosquitto.service
//test the installation
mosquitto -v
```
![](https://i.imgur.com/7KkCzin.jpg)
**Enable Remote Access/ Authentication**
On a modifié le fichier mosquitto.conf pour autoriser la connexion pour les utilisateurs authentifiés.
```
sudo mosquitto_passwd -c /etc/mosquitto/passwd wz
sudo nano /etc/mosquitto/mosquitto.conf
//at the top of the file
per_listener_settings true
//end of the file
allow_anonymous false
listener 1883
password_file /etc/mosquitto/passwd 
```
![](https://i.imgur.com/4txeAtJ.jpg)
Enfin, il faut restart Mosquitto.
```
//Restart Mosquitto for the changes to take effect
sudo systemctl restart mosquitto/
/To check if Mosquitto is actually running
sudo systemctl status mosquitto
```
![](https://i.imgur.com/Uwbphzc.jpg)

**Test**
Pour tester d'abord Mosquitto Broker et Client sur Raspberry Pi, on a installé MQTT Client sur Raspberry Pi.
```
sudo apt install -y mosquitto mosquitto-clients
//Run Mosquitto in the background as a daemon
mosquitto -d
//Subscribing to testTopic Topic terminal#1
mosquitto_sub -d -t testTopic -u wz -P 123456
//To publish a sample message to testTopic terminal Window #2
mosquitto_pub -d -t testTopic -m "Hello world!" -u wz -P 123456
//The message “Hello World!” is received in Window #1 as illustrated in the figure above.
```
![](https://i.imgur.com/kOvSLGb.jpg)
Comme ça, nous avons configuré le broker MQTT pour qu'il reçoit les données envoyées par le client ESP32.

![](https://i.imgur.com/r36UfZU.png)
![](https://i.imgur.com/G2stl8f.jpg)
![](https://i.imgur.com/z86x5Js.jpg)

## 3. Création de l'application passerelle sur le Raspberry Pi
Cette partie de code se trouve dans le fichier *app.py*.
On a utilisé Flask pour créer un serveur web, se connecter au serveur MQTT, récupérer les données des capteurs et les afficher sur une page web.
**Importation des bibliothèques nécessaires**
```
import json
from flask import Flask, render_template
import paho.mqtt.client as mqtt
```
**Configuration des paramètres MQTT**
```
mqtt_server = "172.20.10.12"
mqtt_port = 1883
mqtt_user = "wz"
mqtt_password = "123456"
mqtt_topic = "testTopic"
```
Les variables ci-dessus définissent l'adresse IP du serveur MQTT, le port, l'utilisateur et le mot de passe pour se connecter au serveur MQTT, ainsi que le topic auquel s'abonner.

**Connexion au serveur MQTT et récupération des données du capteur**
On a crée un client MQTT, définit les fonctions de rappel on_connect et on_message, puis se connecté au serveur MQTT. La fonction on_message met à jour la variable globale sensor_value avec la valeur du capteur reçue.
```
client = mqtt.Client()
client.username_pw_set(mqtt_user, mqtt_password)
client.on_connect = on_connect
client.on_message = on_message

client.connect(mqtt_server, mqtt_port, 60)
client.loop_start()
```
**Route principale de l'application Flask**
```
@app.route("/")
def index():
    return render_template("index.html", value=sensor_value)
```
La route principale ("/") de l'application Flask renvoie le fichier HTML "index.html" en passant la valeur du capteur comme argument.

## Création du serveur HTTP sur le Raspberry Pi

Nous avons créé une page Web pour le serveur HTTP affichant les données du capteur de l'ESP32.
Cette partie de code se trouve dans le fichier *index.html* dans le répertoire *templates*.
On a d'abord écrit un code HTML pour afficher la valeur du capteur de manière simple et claire, et se met à jour automatiquement pour montrer les dernières données du capteur.

*index.html*
```
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Sensor Value</title>
  </head>
  <body>
    <h1>Sensor Value</h1>
    <p>{{ value }}</p>
    <script>
      setTimeout(function() {
        location.reload();
      }, 5000);
    </script>
  </body>
</html>
```
## Test et Optimisation
### Test
Pour exécuter correctement l'application *app.py* sur Raspberry Pi, il faut d'abord préparer l'environement.
```
sudo apt-get update -y
sudo apt-get install -y python3-paho-mqtt
sudo python3 app.py
```
On voit bien que la valeur du capteur est bien reçu et affihché ser la page Web et se met à jour : 

![](https://i.imgur.com/4uR3OEV.jpg)

![](https://i.imgur.com/Ikd5PUu.jpg)

### Optimisation
On a fait un peu de cherche sur internet pour optimiser l'affichement.
Pour afficher la valeur du capteur actuelle, qui est passée depuis l'application Flask : 
```
<p>Current value: <span id="current_value">{{ value }}</span></p>
```
Pour créer un élément canvas qui sera utilisé pour dessiner le graphique : 
```
<canvas id="myChart" width="400" height="200"></canvas>
```
On a écrit ce code JavaScript pour mettre à jour et afficher les données du capteur. La fonction updateData() récupère les données du capteur depuis l'application Flask en appelant la route "/data", met à jour le texte de la valeur du capteur et ajoute la valeur du capteur au graphique. La fonction setInterval() appelle updateData() toutes les 2 secondes pour mettre à jour les données en temps réel. 
```
function updateData() {
    fetch("/data")
        .then((response) => response.text())
        .then((value) => {
            document.getElementById("current_value").textContent = value;
            addData(myChart, new Date(), parseFloat(value));
        });
}

setInterval(updateData, 2000); // Update data every 2 seconds

```
Dans le fichier app.py, on a ajouté une nouvelle route "/data" pour renvoyer la valeur du capteur en tant que chaîne de caractères.

```
@app.route("/data")
def data():
    return str(sensor_value)
```
On voit enfin la valeur et le chart sur la page Web.
![](https://i.imgur.com/adWwp65.jpg)



## Sources
Voici les sources que nous avons trouvées pour réaliser notre projet et approfondir nos connaissances : 

[Projet exemple : ESP32 MQTT – Publish and Subscribe with Arduino IDE](https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/)

[Install Raspberry Pi OS, Set Up Wi-Fi, Enable and Connect with SSH](https://randomnerdtutorials.com/installing-raspbian-lite-enabling-and-connecting-with-ssh/)

[Install Mosquitto MQTT Broker on Raspberry Pi](https://randomnerdtutorials.com/how-to-install-mosquitto-broker-on-raspberry-pi/)

[Testing Mosquitto Broker and Client on Raspberry Pi](https://randomnerdtutorials.com/testing-mosquitto-broker-and-client-on-raspbbery-pi/)

[How to Connect ESP32 to MQTT Broker](https://iotdesignpro.com/projects/how-to-connect-esp32-mqtt-broker)

[How To Make a Web Application Using Flask in Python 3](https://www.digitalocean.com/community/tutorials/how-to-make-a-web-application-using-flask-in-python-3)

[Chart.js Samples](https://www.chartjs.org/docs/latest/samples/area/line-boundaries.html)