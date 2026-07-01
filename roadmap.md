# RTOS-Based 6-DOF Aircraft Flight Simulator Roadmap

Bu doküman, projenin başından sonuna kadar sıfırdan tamamen çalışan bir uçuş simülatörüne dönüşmesi için gereken tüm adımları detaylandırmaktadır. 

## Aşama 1: Temel Matematik Kütüphanelerinin Kurulumu
- [x] 3 Boyutlu uzay hesaplamaları için `Vector3` sınıfının oluşturulması (Toplama, çıkarma, skaler çarpım, cross/dot product vs.).
- [x] Rotasyon ve yönelim hesaplamaları için `Quaternion` sınıfının oluşturulması (Euler açılarından dönüşüm, quaternion çarpımı vs.).

## Aşama 2: Sistem Mimarisi ve İskeletin Kurulması
- [x] Durum makinesi (State Machine) tanımlamalarının yapılması (`INIT_BOOT`, `IDLE`, `TAKEOFF`, `CRUISE`, `FAIL_SAFE`).
- [x] `Aircraft` ana sınıfının iskeletinin oluşturulması (Kütle, kanat alanı, sürtünme/kaldırma katsayıları gibi özellikler).
- [x] Çoklu iş parçacığı (Multithreading) temelinin atılması (Physics, Control ve Telemetry thread'lerinin tanımlanması).
- [x] Veri senkronizasyonu için `std::mutex` ve `std::atomic` yapılarının eklenmesi.
- [x] `BootSystem` ve `Shutdown` fonksiyonlarının threadleri başlatıp/bitirecek şekilde yazılması.

## Aşama 3: Thread Döngüleri ve Zamanlama
- [x] `main.cpp` içerisindeki uçak objesinin `BootSystem` ile ayağa kaldırılması ve ana thread'in belirli bir süre çalışıp `Shutdown` edebilmesi için bir yaşam döngüsünün kurulması.
- [x] Thread döngülerinin (`PhysicsLoop`, `ControlLoop`, `TelemetryLoop`) mimaride belirtilen frekanslarda (sırasıyla 1000Hz, 100Hz, 10Hz) çalışmasını sağlayacak "hassas bekleme (precise timing/sleeping)" mekanizmasının kurulması.

## Aşama 4: Fizik Motorunun Geliştirilmesi
- [x] Euler integrasyonu kullanılarak ivmeden hız, hızdan konum hesaplamalarının kodlanması.
- [x] Aerodinamik kuvvetlerin hesaplanması: Lift (Kaldırma kuvveti) ve Drag (Sürüklenme kuvveti).
- [x] Thrust (İtme) ve Gravity (Yerçekimi) kuvvetlerinin uçağın yönelimine (Quaternion) göre vektörel olarak sisteme uygulanması.
- [x] Tüm bu fiziksel durum güncellemelerinin `stateMutex` kullanılarak thread-safe (güvenli) bir şekilde Shared Memory'ye (Paylaşımlı belleğe) yazılması.

## Aşama 5: Uçuş Kontrol ve Durum Yönetimi
- [x] Uçuş durumları arası geçiş mantığının (IDLE -> TAKEOFF -> CRUISE) belirli şartlara (örneğin hıza, irtifaya) göre otomatikleştirilmesi.
- [x] Gaz kolu (Throttle) ve kontrol yüzeylerinin (Aileron, Elevator) durumlarına göre istenilen itme ve yönelim değerlerinin hesaplanması.
- [x] İstenmeyen fiziksel durumlara (örneğin NaN değerli hızlar veya aşırı G kuvveti) karşı `FAIL_SAFE` durumuna geçişi tetikleyecek güvenlik kontrollerinin yazılması.

## Aşama 6: Telemetri ve Loglama
- [x] `stateMutex` kilitlenerek güncel pozisyon, hız, yönelim ve FlightState verilerinin güvenli bir şekilde okunması.
- [x] Okunan bu verilerin, zaman damgasıyla birlikte sonradan analiz edilebilmesi için `.csv` formatında bir dosyaya yazdırılması.

## Aşama 7: Test, Doğrulama ve Analiz (TAMAMLANDI) 🏁
- [x] Sistemin INIT'ten CRUISE aşamasına kadar kesintisiz uçurulması (Bir senaryo çalıştırılması).
- [x] Oluşturulan `.csv` dosyasındaki verilerin (İrtifa - Zaman, Hız - Zaman grafikleri gibi) doğruluğunun ve fizik kurallarına uygunluğunun harici bir araçla (Python/Matplotlib veya Excel) çizdirilerek doğrulanması.

## Aşama 8: Rotasyonel Dinamikler (3D Hareket) (TAMAMLANDI) 🏁
- [x] `Aircraft.h` içerisine Açısal Hız (Angular Velocity), Açısal İvme (Angular Acceleration) ve Atalet Momenti (Inertia) değişkenlerinin eklenmesi.
- [x] Kontrol yüzeylerinin (Aileron, Elevator, Rudder) açılarının tanımlanması ve bunların uçak üzerinde oluşturduğu Tork'un (Döndürme Kuvveti) hesaplanması.
- [x] Hesaplanan tork ile açısal ivme ve açısal hızın Euler ile entegre edilmesi.
- [x] Açısal hız kullanılarak uçağın güncel yönelim (Quaternion) değerinin döndürülerek güncellenmesi.
- [x] ControlLoop içerisinden test manevraları (Örn: Sağa yatma) göndererek uçağın 3D uzayda dönüp dönmediğinin test edilmesi.

## Aşama 9: Web Üzerinden Canlı (Real-Time) 3D Görselleştirme (TAMAMLANDI) 🏁
- [x] Unity yerine `Three.js` ve HTML5 kullanılarak uçuş verilerinin tarayıcı üzerinden 3D animasyonla izlenebilmesi.
- [x] C++ UDP soketi ve Python WebSocket kullanılarak uçağın takla ve irtifa hareketlerinin web ortamına yansıtılması.

## Aşama 10: Masaüstü Telemetri Arayüzü (Qt & C++) (TAMAMLANDI) 🏁
- [x] Uçuş simülatörü klasörü altında Qt projesinin ve GUI'nin oluşturulması.
- [x] Arayüzün donmasını engellemek için Qt Signals & Slots ve QThread mimarisinin kurulması.
- [x] `QUdpSocket` kullanılarak simülatörden gelen gerçek zamanlı UDP uçuş verilerinin (10Hz) dinlenmesi.
- [x] Gelen İrtifa (Altitude) ve Hız (Velocity) verilerinin arayüzde gerçek zamanlı (real-time) grafiklerle (plot/gauge) çizdirilmesi.
