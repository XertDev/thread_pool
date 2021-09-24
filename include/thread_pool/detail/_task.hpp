#ifndef THREAD_POOL__TASK_HPP
#define THREAD_POOL__TASK_HPP


namespace thread_pool::detail
{
	class TaskPimpl
	{
	public:
		virtual void invoke() = 0;
		virtual ~TaskPimpl() = default;
	};

	template<typename F>
	class TaskPimplImpl: public TaskPimpl
	{
	public:
		explicit TaskPimplImpl(F&& fun)
		:
			fun_(std::move(fun))
		{};

		void invoke() final
		{
			fun_();
		}

	private:
		F fun_;
	};

	class Task
	{
	public:
		Task() = default;

		Task(Task&&) = default;
		Task& operator=(Task&&) = default;

		template<typename F>
		requires (!std::same_as<std::decay_t<F>, Task>)
		Task(F&& fun)
		:
			pimpl_(make_pimpl(std::forward<F>(fun)))
		{}

		void operator()()
		{
			pimpl_->invoke();
		}

	private:
		std::unique_ptr<TaskPimpl> pimpl_;

		template<typename FF>
		static auto make_pimpl(FF&& fun)
		{
			using decay_FF = std::decay_t<FF>;
			using impl_t = TaskPimplImpl<decay_FF>;
			return std::make_unique<impl_t>(std::forward<FF>(fun));
		}
	};
}

#endif //THREAD_POOL__TASK_HPP
