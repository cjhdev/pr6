spec = Gem::Specification.new do |s|
    s.name    = "wrangle"
    s.version = "1.0.0"
    s.summary = "PR6 for Ruby"
    s.author  = "Cameron Harper"
    s.email = "contact@cjh.id.com"
    s.homepage = "https://github.com/cjhdev/pr6"
    s.files = Dir.glob("ext/**/*.{c,h,rb}") + Dir.glob("lib/**/*.rb") + Dir.glob("test/**/*.rb") + Dir.glob("db/**/*.{rb}") + ["rakefile"]
    s.extensions = "ext/wrangle/ext_wrangle/extconf.rb"
    s.executables = Dir.entries("bin")
    s.executables.delete(".")
    s.executables.delete("..")
    s.license = 'MIT'
    s.test_files = Dir.glob("test/**/*.rb")
    s.has_rdoc = 'yard'
end
