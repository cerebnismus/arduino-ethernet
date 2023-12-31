<img width="1012" alt="image" src="https://github.com/oguzhan-ince/arduino-ethernet/assets/11842029/3cc8ddc4-c6f9-4905-b1e3-0483c9ff07bc">

### Compile and Run
```bash
$ arduino-cli compile  \
  --fqbn arduino:avr:uno  \
  --port /dev/cu.usbserial-1410  \
  --libraries /Users/macbook/Documents/arduino-ethernet/libs/  \
  --build-cache-path /Users/macbook/Documents/arduino-ethernet/build-cache/   \
  --export-binaries --warnings all  \
  --output-dir /Users/macbook/Documents/arduino-ethernet/bin/   \
  --upload  \
  --verify  \
  --verbose  \
  --clean \
  /Users/macbook/Documents/arduino-ethernet/arduino-ethernet.ino
```

### arduino-ethernet ile ağ yönetimi
Arduino Uno, açık kaynaklı bir mikrodenetleyici kartıdır ve Atmel ATmega328P mikrodenetleyiciyi temel alır. 14 dijital giriş/çıkış pini, 6 analog giriş pini, bir 16 MHz kuartz kristali, bir USB bağlantısı, bir güç jakı ve bir ICSP başlığına sahiptir. Programlama için genellikle Arduino IDE kullanılır, bu sayede C ve C++ dillerinde yazılmış kodlar yüklenir. Arduino Uno, sensörlerden aldığı verileri işleyebilir, motorları kontrol edebilir ve diğer elektronik bileşenleri yönetebilir. Genellikle prototipleme, küçük ölçekli projeler ve eğitim amaçlı kullanılır. Ethernet kartı, bir bilgisayarın veya mikrodenetleyicinin bir Ethernet ağına bağlanmasını sağlar. Bu kartlar genellikle RJ-45 konnektörüne sahiptir ve 10/100/1000 Mbps hızlarını destekler. Arduino için üretilmiş Ethernet kartları (shield), SPI (Serial Peripheral Interface) üzerinden Arduino ile iletişim kurar. Bu sayede, Arduino'nun internete bağlanmasını ve örneğin HTTP istekleri göndermesini veya bir web sunucusu olarak çalışmasını sağlar. Ethernet kartları, IoT projelerinde, uzaktan cihaz kontrolünde ve veri toplama uygulamalarında sıklıkla kullanılır.

Arduino Uno ve Ethernet kartının bir araya getirilmesi, özellikle IoT (Internet of Things) projeleri için ideal bir kombinasyon oluşturur. Bu kombinasyon, sensör verilerinin uzaktan izlenmesi, ev otomasyonu, hava kalitesi monitörleri, otomatik sulama sistemleri ve enerji yönetimi gibi birçok alanda kullanılabilir. Ayrıca, bu bileşenlerin birleştirilmesi, kullanıcıların bir web arayüzü üzerinden Arduino'yu kontrol etmelerini sağlar. Bu sayede, kullanıcılar projelerini internet üzerinden yönetebilir ve veri toplayabilirler. Ayrıca, bu kombinasyon endüstriyel otomasyon, tıbbi izleme sistemleri ve akıllı şehir uygulamaları gibi daha karmaşık projeler için de bir temel oluşturabilir.

### Ethernet Kartı İncelemesi
Proje, Arduino Ethernet kütüphanesi ve W5100Interface temeline dayanmaktadır. Kütüphane, Wiznet W5100 tabanlı Ethernet shield için EthernetInterface'ı uygulayan bir mbed kütüphanesidir. Wiznet W5100 çipi, aynı anda dört bağlantı soketini destekler. Ve asla 4 soketten fazlasını desteklemez. Bu çipin 4 soket sınırlaması, aynı anda birden fazla bağlantı gerektiren projeler için bir kısıtlama oluşturabilir. Örneğin, bir IoT cihazı birden fazla sensörden veri topluyor ve bu verileri farklı sunuculara veya servislere gönderiyorsa, 4 soket sınırlaması bu tür bir uygulamayı kısıtlayabilir. Ayrıca, yüksek hacimli veri trafiği olan veya çok sayıda istemci tarafından erişilen web sunucuları, bu sınırlama nedeniyle performans sorunları yaşayabilir. Real-time (gerçek zamanlı) uygulamalar, video akışı veya VoIP (Ses üzeri IP) gibi projeler de bu sınırlamadan olumsuz etkilenebilir.

Bu tür projeler için birkaç alternatif çözüm yolu mevcuttur:
Soket Yönetimi: Dinamik olarak açılıp kapatılması, yani ihtiyaç duyulduğunda bir soketin kapatılıp yeni bir soketin açılması, kaynakların etkin kullanılmasını sağlar.
Yüksek Performanslı Çipler: Soket desteği sunan başka Ethernet çipleri kullanılabilir. Örneğin, Wiznet'in daha üst modelleri veya farklı üreticilerin çipleri.
Soket Çoğaltma: Aynı soketi kullanarak farklı veri akışlarını multiplexing (çoğaltma) yöntemiyle birleştirmek.
Veri Önbelleği: Gelen verileri önbelleğe almak ve belirli bir zaman diliminde toplu olarak işlemek.
Edge Computing: Veri işleme işlemlerini cihazın kendisinde yaparak, gönderilecek veri miktarını ve dolayısıyla soket ihtiyacını azaltmak.
MQTT gibi Protokoller: Daha az bağlantı ile daha fazla veri göndermeyi ve almayı sağlayan, hafif ağ protokollerini kullanmak.
Bu alternatifler, projenin gereksinimlerine ve kısıtlamalarına bağlı olarak değişkenlik gösterebilir.

### ARP İncelemesi
Address Resolution Protocol (ARP) bir ağ protokolüdür ve bir IP adresinin karşılık gelen Ethernet (MAC) adresini bulmak için kullanılır. ARP, bir cihazın aynı yerel ağdaki diğer cihazlarla iletişim kurabilmesi için gereklidir. Temel işleyişi şu şekildedir: Bir cihaz, bir IP adresine karşılık gelen MAC adresini bilmiyorsa, ARP isteği gönderir. Bu istek, yerel ağdaki tüm cihazlara yayılır. Eğer bir cihaz gönderilen IP adresini taşıyorsa, ARP yanıtı göndererek kendi MAC adresini açıklar. Bu bilgi, isteği gönderen cihazın ARP tablosunda saklanır, böylece gelecekteki iletişim için hızlı bir referans oluşturur.

ARP protokolü, ARP spoofing veya ARP poisoning gibi saldırılara karşı savunmasızdır. Bu tür saldırılarda, saldırgan ARP yanıtlarını sahteleyerek kendini ağdaki bir cihaz gibi gösterebilir. Sonuç olarak, saldırgan ağ trafiğini dinleyebilir veya yönlendirebilir. Bu, kişisel verilerin çalınmasına veya Man-in-the-Middle (MitM) saldırılarına yol açabilir. ARP'nin bu zayıflığı, protokolün güvenlik mekanizmaları olmadan tasarlanmış olmasından kaynaklanır. ARP protokolü için, ARP spoofing saldırılarını önlemek amacıyla dinamik ARP tablo girişlerinin sürekli olarak izlenmesi ve doğrulanması gerekmektedir. Ayrıca, statik ARP girişleri kullanmak da bir seçenek olabilir, ancak bu yöntem ölçeklenebilirlik sorunları yaratabilir. Ağ cihazları üzerinde güvenlik özellikleri, örneğin DHCP Snooping ve Dynamic ARP Inspection, etkinleştirilebilir. Bu özellikler, yalnızca güvenilir cihazların ARP yanıtları göndermesini sağlar.

### ICMP İncelemesi
Internet Control Message Protocol (ICMP), IP protokolü ile birlikte çalışan bir yardımcı protokoldür ve temel olarak hata mesajları ve ağ teşhis işlemleri için kullanılır. ICMP, örneğin "ping" ve "traceroute" gibi ağ teşhis araçlarının temelini oluşturur. Ping, bir ICMP Echo Request mesajı gönderir ve karşılığında bir Echo Reply mesajı bekler. Bu, iki cihaz arasındaki iletişimin sağlıklı olup olmadığını kontrol etmek için kullanılır. Traceroute ise, bir paketin hedefe ulaşana kadar geçtiği yolları ve gecikmeleri göstermek için ICMP mesajlarını kullanır. ICMP, ayrıca ağdaki hatalar ve kesintiler hakkında bilgi sağlar; örneğin, bir hedefe ulaşılamadığında "Destination Unreachable" mesajı gönderilir.

ICMP protokolü de güvenlik açısından bazı zayıf yönler taşır. Örneğin, ICMP flood saldırıları, bir hedefin aşırı sayıda ICMP Echo Request paketi almasına neden olarak servis dışı bırakılmasına yol açabilir. Ayrıca, ICMP'nin hata mesajları saldırganlar tarafından ağın topolojisi hakkında bilgi toplamak için kullanılabilir. ICMP Redirect saldırıları da, ağ trafiğini yanlış yönlendirmek için kullanılabilir. Bu tür saldırılar, ağın güvenliğini tehlikeye atabilir ve hassas verilerin sızmasına neden olabilir. ICMP için, ağ cihazlarının yapılandırmasında belirli ICMP mesaj türlerinin engellenmesi veya sınırlanması mümkündür. Örneğin, ICMP Echo Request mesajlarını sadece güvendiğiniz IP adreslerinden kabul etmek gibi. Ayrıca, ağ izleme araçları kullanılarak anormal ICMP trafiği tespit edilebilir. Bu, olası bir saldırıyı erken bir aşamada belirlemeye yardımcı olabilir. Güvenlik duvarları ve IDS (Intrusion Detection Systems) sistemleri, ICMP trafiğini izlemek ve filtrelemek için de kullanılabilir.
