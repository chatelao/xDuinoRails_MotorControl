#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::time::khz;
use embassy_stm32::timer::simple_pwm::{PwmPin, SimplePwm};
use embassy_stm32::timer::Channel;
use embassy_time::{Duration, Timer, Instant};
use micromath::F32Ext;
use {defmt_rtt as _, panic_probe as _};

#[embassy_executor::main]
async fn main(_spawner: Spawner) {
    info!("Starting Sine Wave Controller for G431");

    let p = embassy_stm32::init(Default::default());

    // Configure Pins for PWM
    // PA8 is TIM1_CH1 (Arduino D7)
    // PA9 is TIM1_CH2 (Arduino D8)
    let ch1 = PwmPin::new_ch1(p.PA8, embassy_stm32::gpio::OutputType::PushPull);
    let ch2 = PwmPin::new_ch2(p.PA9, embassy_stm32::gpio::OutputType::PushPull);

    // Initialize PWM on TIM1 at 20kHz
    let mut pwm = SimplePwm::new(
        p.TIM1,
        Some(ch1),
        Some(ch2),
        None,
        None,
        khz(20),
        Default::default(),
    );

    // Enable channels
    pwm.enable(Channel::Ch1);
    pwm.enable(Channel::Ch2);

    let max_duty = pwm.get_max_duty();
    info!("PWM initialized. Max duty: {}", max_duty);

    let start_time = Instant::now();
    let period_ms = 2500.0;

    loop {
        let now = Instant::now();
        let elapsed_ms = now.duration_since(start_time).as_millis() as f32;

        // Calculate sine wave angle
        // sineValue = sin(2 * PI * (elapsedTime / SINE_WAVE_PERIOD));
        let angle = 2.0 * core::f32::consts::PI * (elapsed_ms / period_ms);
        let sine_value = angle.sin();

        // Map sine value (-1.0 to 1.0) to PWM duty cycle (25% to 75%)
        // 50% duty cycle center, +/- 25% amplitude
        let target_duty = (0.5 + (sine_value * 0.25)) * (max_duty as f32);
        let target_duty_u16 = target_duty as u16;

        // Set PWM
        // Motor Direction Forward: A (Ch1) = PWM, B (Ch2) = 0
        pwm.set_duty(Channel::Ch1, target_duty_u16);
        pwm.set_duty(Channel::Ch2, 0);

        // Log every ~1 second (approx) to avoid spamming, or just debug logging
        // info!("PWM: {}", target_duty_u16);

        Timer::after(Duration::from_millis(10)).await;
    }
}
