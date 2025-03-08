#include "arduino_secrets.h"
/* 
  Sketch criada pelo Arduino IoT Cloud Thing "Untitled"

  Descrição das variáveis da nuvem IoT do Arduino

  As seguintes variáveis são geradas e atualizadas automaticamente quando são feitas alterações no "Thing"

  String horarioRega;
  int nCiclos;
  int umidade;
  int temperatura;

  As variáveis que estão marcadas como READ/WRITE no Cloud Thing também terão funções
  que são chamadas quando seus valores são alterados a partir do Dashboard.
  Essas funções são geradas com o "Thing" e adicionadas no final deste esboço.
*/

#include "thingProperties.h"
#include "DHT11.h"

DHT11 dht11(18); //cria uma instancia do dht11
int ledPin = 2; // Pino do LED (geralmente o BUILTIN_LED)
int bombaPin = 13; 
unsigned long tempoInicio, tempoAtual, tempoInicialBomba, leitura_antes, ultima_tentativa;
bool primeira = 1;

#define tempoAcaoBomba 10000  // Tempo que a bomba deve ficar ligada
#define periodoBomba   432000000  //432000000 Aqui definimos de quanto em quanto tempo vamos aguar as plantas 


time_t UnixTime;

//leitura de sensor em intervalo definido em segundos
unsigned long ler_sensor(unsigned long tempo_agora, unsigned long tempo_antes, int intervalo){
  intervalo = intervalo *1000;
  if(tempo_agora-tempo_antes > intervalo){
    int debug_code = dht11.readTemperatureHumidity(temperatura,umidade);
    Serial.println("TEMPERATURA: "+String(temperatura)+"C°");
    Serial.println("UMIDADE: "+String(umidade)+"%");
    return tempo_agora;
  }else{
    return tempo_antes;
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(bombaPin, OUTPUT);
  Serial.begin(115200);
  tempoInicio = millis();
  leitura_antes = millis();
  nCiclos = 0;
  
  // Garantir inicializacao da serial
  delay(1500); 

  // Iniciar configuracoes de vairavies do Arduino Cloud
  initProperties();

  // Conectar ao Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  /*
     A função a seguir permite que você obtenha mais informações
     relacionadas ao estado da rede e à conexão e aos erros da nuvem IoT
     quanto maior o número, mais detalhadas serão as informações que você obterá.
     O padrão é 0 (somente erros).
     O máximo é 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  leitura_antes = ler_sensor(millis(),leitura_antes,5);

  tempoAtual = millis();
  Serial.println("TEMPO: "+String(tempoAtual-tempoInicio)+"ms");

  // condicao para hora de ativar a bomba de agua periodoBomba-tempo_primeira
  if (tempoAtual-tempoInicio > periodoBomba || (primeira == 1 && tempoAtual > 30000)){ 
    Serial.println("REGA"); 
    primeira = 0;
    int errcount = 0;

    do {
      UnixTime = ArduinoCloud.getLocalTime(); // necessário para ficar legível
      Serial.print("get Local time -> try ");
      Serial.println(errcount);
      while(millis()-ultima_tentativa < 60000){

      }
      ultima_tentativa = millis();
      errcount++;
    }while(UnixTime == 0 && errcount <5);

    horarioRega =ctime(&UnixTime);
    Serial.println("Horario: "+ horarioRega);
    nCiclos++;
    tempoInicialBomba = millis();
    bool done = false;
    while(millis()-tempoInicialBomba < tempoAcaoBomba){
      if(done == false){
        digitalWrite(ledPin, HIGH);
        digitalWrite(bombaPin, HIGH);
        done = true;
      }
    }
    tempoInicio=tempoAtual;
    Serial.println("Numero de vezes:"+String(nCiclos));


  if(tempoAtual-tempoInicio<0){ // Tratamento para overflow, vai acontecer após 40,7 dias
    Serial.println("Overflow identificado");
    Serial.println("Valor tempoAtual: "+String(tempoAtual));
    tempoInicio  = 0;

  }
  }else{
    digitalWrite(ledPin, LOW);
    digitalWrite(bombaPin, LOW);
  }

}




