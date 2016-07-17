spec = Gem::Specification.new do |s|
    s.name    = "wrangle"
    s.version = "1.0.0"
    s.summary = "PR6 for Ruby"
    s.author  = "Cameron Harper"
    s.files = Dir.glob("ext/**/*.{c,h,rb}") + Dir.glob("lib/**/*.rb") + Dir.glob("test/**/*") + ["rakefile"]
    s.extensions = "ext/ext_wrangle/extconf.rb"
    s.executables = Dir.entries("bin")
    s.executables.delete(".")
    s.executables.delete("..")
    s.license = 'MIT'
    s.test_files = Dir.glob("test/**/*.rb")
    s.has_rdoc = 'yard'
end
