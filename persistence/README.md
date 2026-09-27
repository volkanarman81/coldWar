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
Gerçek oyunda nasıl deneneceği için aşağıdaki bölüme bakın.

## Gerçek oyunda deneme

### 1. Derleme

**Windows:**
- **Gerekenler:** LLVM/Clang, Visual Studio Build Tools (Windows SDK için), CMake,
  Ninja ve vcpkg. vcpkg'yi kurup `VCPKG_ROOT` ortam değişkenini onun klasörüne
  ayarlayın.
- **Derleme komutları** (deponun kök klasöründe):
  ```
  cmake --preset win-x64-clang-rwdi
  cmake --build build/win-x64-clang-rwdi
  ```
- **Çıktı:** Oyun `dist\x64-win-rwdi\PoseidonGame.exe` gibi bir klasöre kopyalanır.

**Linux:** Aynı adımlar, preset adı `linux-x64-clang-rwdi`.

Preset'ler derlemeyi `ccache` ile çalıştırır. Kurulu değilse ya kurun ya da ilk
komuta `-DCMAKE_C_COMPILER_LAUNCHER= -DCMAKE_CXX_COMPILER_LAUNCHER=` ekleyin.

**LAN'daki herkes aynı derlemeyi kullanmalı.** İstemciler de yeni komutlara (`isNil`
gibi) ihtiyaç duyar.

### 2. Oyun verisi ve çalıştırma

- **Veri:** Oyun verisi depoda yok. Steam'deki ücretsiz Demo'yu ya da tam oyunun
  verisini kullanın. Demo verisinde editörün ve MP'nin ne kadarının açık olduğu
  bilinmiyor; tam oyun verisi varsa onu tercih edin.
- **Başlatma:** Oyunu veri klasörünü göstererek, log'u da bir dosyaya yazdırarak
  başlatın:
  ```
  PoseidonGame.exe -C "C:\...\oyun verisi klasörü" --log-file persist_test.log
  ```

### 3. Test görevi

1. **Editörde yerleştirin:**
   - 2 oynanabilir asker: `p1` (siz) ve `p2`.
   - Bir araç: `truck1`.
2. **Görevi MP klasörüne koyun:** Görevi kaydedip klasörünü şuraya kopyalayın:

   | Sistem | MP görev klasörü |
   |---|---|
   | Windows | `Belgeler\Cold War Assault\MPMissions\` |
   | Linux | `~/.local/share/Cold War Assault/MPMissions/` |

3. **Çerçeveyi ekleyin:** Görev klasörüne `mission-template/persistence/` klasörünü
   kopyalayın. `init.sqs`, `initJIP.sqs` ve `description.ext` satırlarını da ekleyin
   (bkz. [Kurulum](#kurulum)).
4. **Ayarları yapın:** `persistence/config.sqf` içinde:
   ```sqf
   PERS_slots = ["p1", "p2"];
   PERS_vehicles = ["truck1"];
   ```

### 4. Deneme senaryosu

1. **LAN oyunu açın:** Multiplayer'dan LAN oyunu açıp görevi başlatın. Log'da
   `Persistence: host ready, 0 saved players` satırı görünmeli.
2. **Durumu değiştirin:** Yürüyün, bir silah bırakın, biraz hasar alın, kamyonu
   başka yere götürün.
3. **Kaydı bekleyin:** En az 60 saniye bekleyin. Sonra kayıt dosyasının
   oluştuğunu kontrol edin (konumu için bkz. [Kayıt dosyası](#kayıt-dosyası)).
   Dosya düz metindir, içinde konumunuzu görebilirsiniz.
4. **Yeniden başlatın:** Oyunu kapatıp aynı görevi tekrar host edin. Konum,
   teçhizat, sağlık, kamyon ve saat kaldığı yerden gelmeli.
5. **Oyun sürerken katılma:** İkinci bir kişi oyun sürerken katılsın, oynayıp
   çıksın, sonra tekrar katılsın. Kaldığı yerden devam etmeli.

**Tek bilgisayarda deneme:** İkinci bir oyun penceresini farklı bir adla açıp kendi
oyununuza bağlanmayı deneyebilirsiniz. Aynı makinede iki kopyanın birlikte
çalıştığı doğrulanmadı.
```
PoseidonGame.exe -C "<veri klasörü>" --name Test2 --connect 127.0.0.1
```

### Sorun olursa

Şu iki dosya sorunu bulmak için yeterlidir:
- **Log dosyası:** `--log-file` ile verdiğiniz dosya. Bu seçeneği vermediyseniz
  ve oyunu konsolsuz başlattıysanız `Belgeler\Cold War Assault\logs\cwr_*.log`.
- **Kayıt dosyası:** `Saves\` altındaki `.txt` dosyası.
