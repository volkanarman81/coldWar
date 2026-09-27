# Kalıcı dünya (persistence) çerçevesi

LAN'da oyun içinden host edilen görevlerde ilerlemeyi oturumlar arasında korur.
Oyunu kapatıp ertesi gün aynı görevi açtığınızda herkes kaldığı yerden devam eder.

| Ne kaydediliyor | Ayrıntı |
|---|---|
| Oyuncular | Profil adına göre: konum, yön, sağlık, silahlar, şarjörler, elindeki silah |
| Araçlar | Listelediğiniz araçların konumu, yönü, hasarı, yakıtı; yok edilen araç sonraki oturumlarda da yok |
| Dünya | Tarih/saat, bulut, sis, yağmur |
| Görev değişkenleri | Listelediğiniz global değişkenler (sayı, metin, true/false ve bunların dizileri) |

Kayıt her 60 saniyede bir host'un diskine yazılır. Oyuncular durumlarını her 15
saniyede host'a bildirir. Yani ani bir kapanmada en fazla yaklaşık bir dakikalık
ilerleme kaybolur.

## Gereksinim

Bu depodaki değiştirilmiş motor: çerçeve yeni `saveString`, `loadString`, `str`,
`isNil`, `date`, `overcast`, `fog` ve `rain` komutlarını kullanır. Orijinal oyunda
çalışmaz. Herkesin aynı sürümü kullanması gerekir.

## Kurulum

1. `mission-template/persistence/` klasörünü görevinizin klasörüne kopyalayın
   (ör. `Missions/Kampanyam.Eden/persistence/`).
2. Editörde her oynanabilir birime bir isim verin (`p1`, `p2`, ...).
   Kaydedilecek araçlara da isim verin (`kamyon1`, `tank1`, ...).
3. `persistence/config.sqf` dosyasını düzenleyin:
   ```sqf
   PERS_saveName = "kampanyam.txt";           // her görev için farklı bir ad
   PERS_slots = ["p1", "p2", "p3", "p4"];      // tüm oyuncu slotları
   PERS_vehicles = ["kamyon1", "tank1"];
   PERS_vars = ["para", "alinanKasabalar"];
   ```
4. Görevin `init.sqs` dosyasına şu satırı ekleyin, aynı satırla bir `initJIP.sqs`
   oluşturun:
   ```
   [] exec "persistence\init.sqs"
   ```
5. `description.ext` içine oyuna sonradan katılmayı açan satırı ekleyin:
   ```
   joinInProgress = 1;
   ```
   Hazır örnekler `mission-template/` içinde.

Görev değişkenlerinin varsayılan değerini, kayıttan gelen değeri ezmeyecek şekilde
verin:
```
?isNil "para" : para = 0
```

## LAN'da oynamak

1. Host eden kişi **Multiplayer → New** ile LAN oyunu açar ve görevi seçer.
2. Diğerleri aynı ağdan oyuna katılır. Oyun sürerken katılmak da mümkündür.
3. Bitirirken host oyunu kapatır. Son otomatik kayıt diskte kalır.

Dikkat edilecekler:

- **Kayıtları host tutar.** Her seferinde aynı kişi, aynı profil adıyla host etmeli.
  Başka biri host ederse o kişinin bilgisayarında kayıt yoktur ve görev baştan başlar.
- **Profil adları farklı olmalı.** Oyuncular profil adıyla eşleştirilir. İki kişi
  "Player" adını kullanırsa aynı kaydı paylaşırlar.
- İstemciler kendi diskine hiçbir şey yazmaz. MP'de `saveString` yalnızca host'ta
  çalışır.

## Kayıt dosyası

Host'un profil klasöründe:

| Sistem | Konum |
|---|---|
| Windows | `%APPDATA%\CWR\Users\<profil adı>\Saves\` |
| Linux | `~/.config/CWR/Users/<profil adı>/Saves/` |

- `kampanyam.txt`: güncel kayıt. Düz metindir, okunabilir.
- `kampanyam.txt.bak`: bu oturumun başladığı kayıt. Kötü giden bir oturumu geri
  almak için oyunu kapatın ve `.bak` dosyasını `kampanyam.txt` üzerine kopyalayın.
- **Sıfırlamak** için oyunu kapatıp `kampanyam.txt` dosyasını silin.
- Kayıt önce geçici bir dosyaya yazılıp sonra yerine taşınır. Oyun yazma sırasında
  çökse bile yarım kalmış bir kayıt oluşmaz.

Elle kaydetmek için (ör. görev sonu tetikleyicisinde): `[] call PERS_fnc_save`

## Sınırlar

- Yarım kalmış şarjörler dolu olarak geri gelir (motorda mermi sayısını ayarlayan
  bir komut yok).
- Araç içindeki oyuncu, geri yüklemede aracın 3 m yanına konur.
- Yalnızca `PERS_vehicles` içinde adı olan araçlar kaydedilir; oyun sırasında
  `createVehicle` ile oluşturulanlar kaydedilmez. Araç kargosu da kaydedilmez.
- AI birimleri kaydedilmez.
- Değişkenlerde birim/araç/grup referansları saklanamaz.
- Ölüp yeniden doğan oyuncu, sonraki bildirimde yeni (varsayılan) teçhizatıyla
  kaydedilir.
- Tek oyunculu mod desteklenmez. Tek başınıza test etmek için de LAN oyunu açın.

## Nasıl çalışır

1. Host görev başında kaydı okur, dünyayı, araçları ve değişkenleri uygular.
2. Her oyuncu, fonksiyonlarını yükledikten sonra host'a "hazırım" der
   (`PERS_hello_<slot>`). Host o oyuncunun kaydını `remoteExec` ile doğrudan
   oyuncunun kendi makinesine gönderir. `setDir` ve `addWeapon` gibi komutlar
   yalnızca birimin sahibi olan makinede etkili olduğu için uygulama orada yapılır.
3. Geri yükleme bitene kadar oyuncu durum bildirmez. Böylece yeni doğmuş bir birimin
   başlangıç teçhizatı kaydı ezmez.
4. Oyuncular durumlarını `publicVariable` ile düzenli olarak bildirir (`PERS_rep_<slot>`).
   Host bunları toplar ve belirli aralıklarla `str` ile metne çevirip `saveString`
   ile yazar.

## Testler

`tests/run.sh`, `.sqf` dosyalarını değiştirmeden gerçek SQF yorumlayıcısında
çalıştırır. Oyun komutları testte sahte (mock) sürümlerle değiştirilir.

- Üç oturumluk bir senaryoyu dener: kaydet, yeniden başlat, geri yükle, araç içinde
  kayıt, bozuk dosya.
- Tüm `.sqs` satırlarının sözdizimini kontrol eder.

```sh
sudo apt install clang libfmt-dev libspdlog-dev
persistence/tests/run.sh
```

Bu testler gerçek bir LAN oturumunun yerini tutmaz. Ağ davranışı (`remoteExec`,
`publicVariable`) koddan incelenerek tasarlandı ama gerçek bir oyunda denenmedi.
İlk oturumda host'un log'unda `Persistence: host ready` satırını görmelisiniz.
