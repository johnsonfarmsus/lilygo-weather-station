# Contributing to LilyGo Weather Station

Thank you for considering contributing to this project! 🎉

## How to Contribute

### Reporting Bugs

If you find a bug, please open an issue with:
- Description of the bug
- Steps to reproduce
- Expected behavior
- Actual behavior
- Screenshots (if applicable)
- Your hardware version (T-Display S3 model)
- Serial monitor output (if relevant)

### Suggesting Features

Feature requests are welcome! Please open an issue describing:
- What you'd like to see
- Why it would be useful
- How you envision it working

### Pull Requests

1. **Fork** the repository
2. **Create a branch** for your feature (`git checkout -b feature/amazing-feature`)
3. **Make your changes**
4. **Test thoroughly** on actual hardware
5. **Commit** with clear messages (`git commit -m 'Add amazing feature'`)
6. **Push** to your branch (`git push origin feature/amazing-feature`)
7. **Open a Pull Request**

### Code Guidelines

- Follow existing code style and formatting
- Add comments for complex logic
- Test on real hardware before submitting
- Keep commits focused and atomic
- Update documentation if needed

### Testing

Before submitting a PR, please test:
- Compile succeeds without warnings
- Uploads successfully to hardware
- WiFi connection works
- API calls succeed
- Display shows correct data
- No memory leaks or crashes

## Development Setup

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/lilygo-weather-station.git
cd lilygo-weather-station

# Create a branch
git checkout -b feature/my-feature

# Make changes, then test
pio run --target upload
pio device monitor
```

## Areas for Contribution

Some ideas for contributions:
- **Weather icons**: Add graphical icons for weather conditions
- **Touch support**: Implement touch controls
- **Multiple locations**: Support switching between cities
- **Hourly forecast**: Add detailed hourly view
- **Battery display**: Show battery level
- **Dark mode**: Toggle between themes
- **Configuration UI**: Web-based setup page
- **Alerts**: Show weather warnings/alerts
- **Graphs**: Historical temperature graphs
- **More data**: Wind, pressure, UV index, etc.

## Questions?

Feel free to open an issue for any questions about contributing!

---

Thank you for helping make this project better! 🌤️

