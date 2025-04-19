// 使用Web Audio API生成8位风格的游戏音效
const fs = require('fs');
const { AudioContext } = require('web-audio-api');

// 创建音频上下文
const audioContext = new AudioContext();
const sampleRate = audioContext.sampleRate;

// 生成玩家射击音效
function generatePlayerShootSound() {
  const duration = 0.15; // 持续时间（秒）
  const frameCount = sampleRate * duration;
  const audioBuffer = audioContext.createBuffer(1, frameCount, sampleRate);
  const channelData = audioBuffer.getChannelData(0);
  
  // 生成一个简单的下降音调
  for (let i = 0; i < frameCount; i++) {
    const t = i / sampleRate;
    const frequency = 800 - 2000 * t; // 从800Hz降到400Hz
    channelData[i] = 0.5 * Math.sin(2 * Math.PI * frequency * t);
    
    // 应用简单的音量包络（淡出）
    channelData[i] *= 1 - t / duration;
  }
  
  return audioBuffer;
}

// 生成外星人射击音效
function generateAlienShootSound() {
  const duration = 0.2; // 持续时间（秒）
  const frameCount = sampleRate * duration;
  const audioBuffer = audioContext.createBuffer(1, frameCount, sampleRate);
  const channelData = audioBuffer.getChannelData(0);
  
  // 生成一个简单的上升音调
  for (let i = 0; i < frameCount; i++) {
    const t = i / sampleRate;
    const frequency = 200 + 500 * t; // 从200Hz升到700Hz
    channelData[i] = 0.5 * Math.sin(2 * Math.PI * frequency * t);
    
    // 应用简单的音量包络（淡出）
    channelData[i] *= 1 - t / duration;
  }
  
  return audioBuffer;
}

// 生成爆炸音效
function generateExplosionSound() {
  const duration = 0.4; // 持续时间（秒）
  const frameCount = sampleRate * duration;
  const audioBuffer = audioContext.createBuffer(1, frameCount, sampleRate);
  const channelData = audioBuffer.getChannelData(0);
  
  // 生成噪声爆炸
  for (let i = 0; i < frameCount; i++) {
    const t = i / sampleRate;
    // 白噪声 + 一些低频内容
    channelData[i] = (Math.random() * 2 - 1) * 0.6;
    
    // 添加低频内容
    channelData[i] += 0.4 * Math.sin(2 * Math.PI * 80 * t);
    
    // 应用音量包络
    channelData[i] *= Math.exp(-4 * t);
  }
  
  return audioBuffer;
}

// 导出为WAV文件
function saveAudioBufferToWav(audioBuffer, filename) {
  console.log(`Generating ${filename}...`);
  
  // 获取PCM数据
  const pcmData = audioBuffer.getChannelData(0);
  
  // 创建WAV文件数据
  const wavData = new ArrayBuffer(44 + pcmData.length * 2);
  const view = new DataView(wavData);
  
  // 写入WAV文件头
  writeString(view, 0, 'RIFF');
  view.setUint32(4, 36 + pcmData.length * 2, true);
  writeString(view, 8, 'WAVE');
  writeString(view, 12, 'fmt ');
  view.setUint32(16, 16, true); // 子块大小
  view.setUint16(20, 1, true); // 编码格式（PCM）
  view.setUint16(22, 1, true); // 声道数
  view.setUint32(24, sampleRate, true); // 采样率
  view.setUint32(28, sampleRate * 2, true); // 每秒字节数
  view.setUint16(32, 2, true); // 每个样本的字节数
  view.setUint16(34, 16, true); // 每个样本的位数
  writeString(view, 36, 'data');
  view.setUint32(40, pcmData.length * 2, true); // 数据子块大小
  
  // 写入PCM数据
  floatTo16BitPCM(view, 44, pcmData);
  
  // 将ArrayBuffer保存为文件
  const buffer = Buffer.from(wavData);
  fs.writeFileSync(filename, buffer);
  console.log(`Saved ${filename}`);
}

// 辅助函数：写入字符串
function writeString(view, offset, string) {
  for (let i = 0; i < string.length; i++) {
    view.setUint8(offset + i, string.charCodeAt(i));
  }
}

// 辅助函数：将浮点数转换为16位PCM
function floatTo16BitPCM(view, offset, input) {
  for (let i = 0; i < input.length; i++, offset += 2) {
    const s = Math.max(-1, Math.min(1, input[i]));
    view.setInt16(offset, s < 0 ? s * 0x8000 : s * 0x7FFF, true);
  }
}

// 生成并保存所有音效
console.log('Generating game sound effects...');

// 玩家射击音效
const playerShootBuffer = generatePlayerShootSound();
saveAudioBufferToWav(playerShootBuffer, 'sounds/player_shoot.wav');

// 外星人射击音效
const alienShootBuffer = generateAlienShootSound();
saveAudioBufferToWav(alienShootBuffer, 'sounds/alien_shoot.wav');

// 爆炸音效
const explosionBuffer = generateExplosionSound();
saveAudioBufferToWav(explosionBuffer, 'sounds/explosion.wav');

console.log('All sound effects generated successfully!'); 